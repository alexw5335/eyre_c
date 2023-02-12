#include "internal.h"
#include "nodes.h"
#include "symbols.h"
#include "enums.h"
#include "encodings.h"
#include <intrin.h>
#include "buffer.h"



// Variables



typedef void (*CustomEncoding)(InsNode* node);
static SrcFile* srcFile;
static int pos;
static EyreGroup* group;
static CustomEncoding customEncodings[MNEMONIC_COUNT];



// Errors



static void assemblerErrorAt(char* format, int offset, char* file, int line, ...) {
	if(pos - offset >= srcFile->nodeCount)
		error("Invalid assembler error node index: %d", pos - offset);
	fprintf(stdout, "Assembler error at %s:%d: ", srcFile->path, srcFile->nodeLines[pos - offset]);
	va_list args;
	va_start(args, line);
	vfprintf(stdout, format, args);
	fprintf(stdout, "\n");
	errorAt("Assembler error", file, line);
}

#define assemblerErrorOffset(format, offset, ...) assemblerErrorAt(format, offset, __FILE__, __LINE__, ##__VA_ARGS__)

#define assemblerError(format, ...) assemblerErrorAt(format, 1, __FILE__, __LINE__, ##__VA_ARGS__)

#define invalidEncoding() assemblerError("Invalid encoding")



// Immediate resolution



static int hasImmReloc = 0;

static int immRelocCount = 0;



static int resolveImmRec(void* n, int regValid) {
	char type = *(char*) n;

	if(type == NODE_INT) {
		IntNode* node = n;
		return node->value;
	}

	if(type == NODE_UNARY) {
		UnaryNode* node = n;
		return calcUnaryInt(
			node->op,
			resolveImmRec(node->value, regValid & (node->op == UNARY_POS))
		);
	}

	if(type == NODE_BINARY) {
		BinaryNode* node = n;
		return calcBinaryInt(
			node->op,
			resolveImmRec(node->left, regValid & ((node->op == BINARY_ADD) | (node->op == BINARY_SUB))),
			resolveImmRec(node->right, regValid & (node->op == BINARY_ADD))
		);
	}

	if(type == NODE_SYM) {
		SymNode* node = n;
		SymBase* symBase = node->symbol;

		if(symBase->flags & SYM_FLAGS_POS) {
			if(immRelocCount == 0 && !regValid)
				assemblerError("First relocation (absolute or relative) must not be negative nor multiplied");
			immRelocCount++;
			hasImmReloc = TRUE;
			return 0;
		} else {
			assemblerError("Invalid symbol");
		}
	}

	assemblerError("Invalid node");

	return 0;
}



static int resolveImm(void* n) {
	hasImmReloc = 0;
	immRelocCount = 0;
	char type = *(char*) n;
	if(type == NODE_IMM) n = ((ImmNode*)n)->value;
	return resolveImmRec(n, 1);
}



// Memory resolution



static int baseReg = -1;

static int indexReg = -1;

static int indexScale = 0;

static int aso = -1; // -1: No reg present. 0: Both R64. 1: Both R32.

static int memRelocCount = 0;

static int hasMemReloc = 0;



static void checkAso(int width) {
	if(width == WIDTH_DWORD)
		if(aso == 0)
			assemblerError("Invalid effective address");
		else
			aso = 1;
	else if(width == WIDTH_QWORD)
		if(aso == 1)
			assemblerError("Invalid effective address");
		else
			aso = 0;
	else
		assemblerError("Invalid effective address");
}



static int resolveMemRec(void* n, int regValid) {
	EyreNodeType type = nodeType(n);

	if(type == NODE_UNARY) {
		UnaryNode* node = n;
		return calcUnaryInt(node->op, resolveMemRec(node->value, regValid & (node->op == UNARY_POS)));
	}

	if(type == NODE_INT) {
		IntNode* node = n;
		return node->value;
	}

	if(type == NODE_REG) {
		RegNode* node = n;

		if(!regValid)
			assemblerError("Invalid effective address");

		checkAso(node->width);

		if(baseReg >= 0) {
			if(indexReg >= 0)
				assemblerError("Invalid effective address");

			indexReg = node->value;
			indexScale = 1;
		} else {
			baseReg = node->value;
		}

		return 0;
	}

	if(type == NODE_BINARY) {
		BinaryNode* node = n;
		EyreNodeType leftType = nodeType(node->left);
		EyreNodeType rightType = nodeType(node->right);

		if(node->op == BINARY_MUL) {
			if(leftType == NODE_REG && rightType == NODE_INT) {
				if(indexReg >= 0 || !regValid)
					assemblerError("Invalid effective address");
				checkAso(((RegNode*) node->left)->width);
				indexReg = ((RegNode*) node->left)->value;
				indexScale = ((IntNode*) node->right)->value;
				return 0;
			} else if(leftType == NODE_INT && rightType == NODE_REG) {
				if(indexReg >= 0 || !regValid)
					assemblerError("Invalid effective address");
				checkAso(((RegNode*) node->right)->width);
				indexReg = ((RegNode*) node->right)->value;
				indexScale = ((IntNode*) node->left)->value;
				return 0;
			}
		}

		return calcBinaryInt(
			node->op,
			resolveMemRec(node->left, regValid & ((node->op == BINARY_ADD) | (node->op == BINARY_SUB))),
			resolveMemRec(node->right, regValid & (node->op == BINARY_ADD))
		);
	}

	if(type == NODE_SYM) {
		SymNode* node = n;
		SymBase* symBase = node->symbol;

		if(symBase->flags & SYM_FLAGS_POS) {
			if(memRelocCount == 0 && !regValid)
				assemblerError("First relocation (absolute or relative) must be positive");
			memRelocCount++;
			hasMemReloc = TRUE;
			return 0;
		}

		assemblerError("Invalid effective address symbol");
	}

	if(type == NODE_IMM) {
		ImmNode* node = n;
		return resolveMemRec(node->value, regValid);
	}

	if(type == NODE_MEM) {
		MemNode* node = n;
		return resolveMemRec(node->value, regValid);
	}

	assemblerError("Invalid effective address node: %s", eyreNodeNames[type]);
	return 0;
}



static int resolveMem(void* n) {
	baseReg       = -1;
	indexReg      = -1;
	indexScale    = 0;
	aso           = -1;
	hasMemReloc   = 0;
	memRelocCount = 0;

	int disp = resolveMemRec(n, 1);

	// RSP and ESP cannot be index registers, swap to base if possible
	if(baseReg >= 0 && indexReg >= 0 && indexReg == 4) {
		if(indexScale != 1)
			assemblerError("Invalid effective address");
		int temp = indexReg;
		indexReg = baseReg;
		baseReg = temp;
	}

	return disp;
}



// Encoding utils



static EyreEncoding* getEncoding(EyreOperands operands) {
	if(group->operandsBits && (1 << operands) == 0)
		assemblerError("Invalid operands");
	int index = _popcnt32(group->operandsBits & ((1 << operands) - 1));
	return &group->encodings[index];
}



static int hasSpecifier(EyreSpecifier specifier) {
	return (group->specifierBits & (1 << specifier)) != 0;
}



static inline int rexw(int width, int widths) {
	return (widths >> 2) & (width == WIDTH_QWORD);
}



static inline int checkO16(int width) {
	if(width == WIDTH_WORD) write8(0x66);
}



static inline int checkWidths(int width, int widths) {
	if(((1 << width) & widths) == 0)
		assemblerError("Invalid encoding");
}



static void writeModRM(int mod, int reg, int rm) {
	write8((mod << 6) | (reg << 3) | rm);
}



static void writeSib(int scale, int index, int base) {
	write8((scale << 6) | (index << 3) | base);
}



static void writeRex(int w, int r, int x, int b) {
	int value = 0b01000000 | (w << 3) | (r << 2) | (x << 1) | b;
	if(value != 0b01000000) write8(value);
}



static void writeOpcode(EyreEncoding* encoding, EyreWidth width, int widths) {
	writeVarLengthInt(encoding->opcode + ((width != WIDTH_BYTE) & (widths >> 2)));
}



static inline void writeOpReg(int opcode, int offset) {
	int length = (7 + _bit_scan_reverse(opcode | 1) & -8) >> 3;
	writeVarLengthInt(opcode + (offset << ((length - 1) << 2)));
}



static inline void checkPrefix(EyreEncoding* encoding) {
	if(encoding->prefix != 0) write8(encoding->prefix);
}



static void addRelocation(char width, void* node, int offset) {
	Relocation* relocation = &relocations[relocationCount++];
	relocation->pos = pos;
	relocation->width = width;
	relocation->node = node;
	relocation->offset = offset;
}



static void addDefaultRelocation(char width, void* node) {
	addRelocation(width, node, -2);
}



static void addAbsoluteRelocation(char width, void* node) {
	addRelocation(width, node, -1);
}



static void addRelativeRelocation(char width, void* node, int offset) {
	addRelocation(width, node, offset);
}



// Base encoding



static void encodeNone(EyreOperands operands) {
	EyreEncoding* encoding = getEncoding(operands);
	checkPrefix(encoding);
	writeVarLengthInt(encoding->opcode);
}



static void encode1R(EyreOperands operands, RegNode* op1) {
	EyreEncoding* encoding = getEncoding(operands);
	int widths = encoding->widths;
	int width = op1->width;
	int value = op1->value;
	checkWidths(width, widths);
	checkO16(width);
	checkPrefix(encoding);
	writeRex(rexw(width, widths), 0, 0, (value >> 3) & 1);
	writeOpcode(encoding, width, widths);
	writeModRM(0b11, encoding->extension, value);
}



static void encode1O(EyreOperands operands, RegNode* op1) {
	EyreEncoding* encoding = getEncoding(operands);
	int widths = encoding->widths;
	int width = op1->width;
	int value = op1->value;
	checkWidths(width, widths);
	checkO16(width);
	checkPrefix(encoding);
	writeRex(rexw(width, widths), 0, 0, (value >> 3) & 1);
	writeOpReg(encoding->opcode, value & 7);
}



static int writeMem(
	int opcode,
	void* n,
	int rexW,
	int rexR,
	int reg,
	int immLength
) {
	int dip   = resolveMem(n);
	int base  = baseReg;
	int index = indexReg;
	int scale = indexScale;

	if(aso == 1) write8(0x67);

	int mod = 0;
}



static void writeImm(InsNode* node, char width, int value) {
	if(hasImmReloc) {
		addDefaultRelocation(width, node);
		bufferAdvance(1 << width);
		return;
	}

	switch(width) {
		case WIDTH_BYTE: {
			if(!isImm8(value)) invalidEncoding();
			write8(value);
			break;
		}

		case WIDTH_WORD: {
			if(!isImm16(value)) invalidEncoding();
			write16(value);
			break;
		}
		case WIDTH_DWORD:
		case WIDTH_QWORD: {
			if(!isImm32(value)) invalidEncoding();
			write32(value);
			break;
		}
	}


}



// Instruction assembly



static void assemble0() {
	encodeNone(OPERANDS_NONE);
}



static void assemble1(InsNode* node) {
	int type1 = nodeType(node->op1);

	if(type1 == NODE_REG) {
		if(hasSpecifier(SPECIFIER_O)) {
			encode1O(OPERANDS_O, (RegNode*) node->op1);
		} else {
			encode1R(OPERANDS_R, (RegNode*) node->op1);
		}
	} else if(type1 == NODE_MEM) {

	} else {
		assemblerError("Invalid encoding");
	}
}



static void assembleInstruction(InsNode* node) {
	group = eyreGetEncodings(node->mnemonic);

	CustomEncoding customEncoding = customEncodings[node->mnemonic];

	if(customEncoding != NULL) {
		customEncoding(node);
		return;
	}

	if(node->op1 == NULL)
		assemble0();
	else if(node->op2 == NULL)
		assemble1(node);
	else if(node->op3 == NULL)
		assemblerError("2-operand instructions not yet supported");
	else if(node->op4 == NULL)
		assemblerError("3-operand instructions not yet supported");
	else
		assemblerError("4-operand instructions not yet supported");
}



// Other nodes



static void handleLabel(LabelNode* node) {
	node->symbol->pos = (int) (bufferPos - (void*) buffer);
}



// Public functions



void eyreAssemble(SrcFile* inputSrcFile) {
	srcFile = inputSrcFile;

	for(int i = 0; i < srcFile->nodeCount; i++) {
		void* n = srcFile->nodes[i];
		char type = *(char*) n;

		if(type == NODE_INS) {
			assembleInstruction(n);
		} else if(type == NODE_LABEL) {
			handleLabel(n);
		}
	}
}



// Custom encoding



static void customEncodeJCC(InsNode* node) {
	if(node->op2 != NULL)
		invalidEncoding();
	int imm = resolveImm(node->op1);

	if(hasImmReloc) {
		encodeNone(OPERANDS_CUSTOM2);
		addRelativeRelocation(WIDTH_DWORD, node, 0);
		write32(0);
	} else if(isImm8(imm)) {
		encodeNone(OPERANDS_CUSTOM1);
		write8(imm);
	} else {
		encodeNone(OPERANDS_CUSTOM2);
		write32(0);
	}
}



static void customEncodeCALL(InsNode* node) {
	if(node->op2 != NULL)
		invalidEncoding();
	int imm = resolveImm(node->op1);
	encodeNone(OPERANDS_CUSTOM1);
	if(hasImmReloc) addRelocation(WIDTH_DWORD, node, 0);
	write32(imm);
}



static void customEncodeINT(InsNode* node) {
	if(node->op2 != NULL || nodeType(node->op1) != NODE_IMM)
		invalidEncoding();

	int imm = resolveImm(node->op1);
	write8(0xCD);
	if(hasImmReloc) addDefaultRelocation(WIDTH_BYTE, node);
	if(!isImm8(imm)) assemblerError("Invalid encoding");
	write8(imm);
}



static void customEncodeRET(InsNode* node) {
	if(node->op1 == NULL) {
		encodeNone(OPERANDS_CUSTOM1);
		return;
	}

	if(node->op2 != NULL || nodeType(node->op1) != NODE_IMM)
		invalidEncoding();

	int imm = resolveImm(node->op1);
	encodeNone(OPERANDS_CUSTOM2);
	if(hasImmReloc) addDefaultRelocation(WIDTH_WORD, node);
	if(!isImm16(imm)) assemblerError("Invalid encoding");
	write16(imm);
}



static void customEncodePUSH(InsNode* node) {
	if(node->op2 != NULL) assemblerError("Invalid encoding");
}



static CustomEncoding customEncodings[MNEMONIC_COUNT] = {
	[MNEMONIC_CALL] = customEncodeCALL,
	[MNEMONIC_JMP]  = customEncodeJCC,
	[MNEMONIC_JA]   = customEncodeJCC,
	[MNEMONIC_JAE]  = customEncodeJCC,
	[MNEMONIC_JB]   = customEncodeJCC,
	[MNEMONIC_JBE]  = customEncodeJCC,
	[MNEMONIC_JC]   = customEncodeJCC,
	[MNEMONIC_JE]   = customEncodeJCC,
	[MNEMONIC_JG]   = customEncodeJCC,
	[MNEMONIC_JGE]  = customEncodeJCC,
	[MNEMONIC_JL]   = customEncodeJCC,
	[MNEMONIC_JLE]  = customEncodeJCC,
	[MNEMONIC_JNA]  = customEncodeJCC,
	[MNEMONIC_JNAE] = customEncodeJCC,
	[MNEMONIC_JNB]  = customEncodeJCC,
	[MNEMONIC_JNBE] = customEncodeJCC,
	[MNEMONIC_JNC]  = customEncodeJCC,
	[MNEMONIC_JNE]  = customEncodeJCC,
	[MNEMONIC_JNG]  = customEncodeJCC,
	[MNEMONIC_JNGE] = customEncodeJCC,
	[MNEMONIC_JNL]  = customEncodeJCC,
	[MNEMONIC_JNLE] = customEncodeJCC,
	[MNEMONIC_JO]   = customEncodeJCC,
	[MNEMONIC_JP]   = customEncodeJCC,
	[MNEMONIC_JPE]  = customEncodeJCC,
	[MNEMONIC_JPO]  = customEncodeJCC,
	[MNEMONIC_JS]   = customEncodeJCC,
	[MNEMONIC_JZ]   = customEncodeJCC,
	[MNEMONIC_INT]  = customEncodeINT,
	[MNEMONIC_RET]  = customEncodeRET,
	[MNEMONIC_RETF] = customEncodeRET
};



// Public functions



void* getAssemblerBuffer() {
	return buffer;
}



int getAssemblerBufferLength() {
	return getBufferLength();
}