#include "internal.h"
#include "nodes.h"
#include "symbols.h"
#include "enums.h"
#include "encodings.h"
#include <intrin.h>
#include "buffer.h"
#include "intern.h"
#include "mnemonics.h"



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



// Utils



static inline int regRex(int reg) {
	return (reg >> 3) & 1;
}



static inline int isSP(int reg) {
	return reg == 4;
}



static inline int isOdd(int value) {
	return value & 1;
}



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

		int left = resolveMemRec(node->left, regValid & ((node->op == BINARY_ADD) | (node->op == BINARY_SUB)));
		int right = resolveMemRec(node->right, regValid & (node->op == BINARY_ADD));
		return calcBinaryInt(node->op, left, right);
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
	if(baseReg >= 0 && indexReg >= 0 && isSP(indexReg)) {
		if(indexScale != 1)
			assemblerError("Invalid effective address");
		int temp = indexReg;
		indexReg = baseReg;
		baseReg = temp;
	}

	switch(indexScale) {
		case 0: case 1: case 2: case 4: case 8:
			break;
		default:
			assemblerError("Invalid index scale: %d", indexScale);
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



static int getOpcode(EyreEncoding* encoding, EyreWidth width, int widths) {
	return encoding->opcode + ((width != WIDTH_BYTE) & (widths & 1));
}



static void writeOpcode(EyreEncoding* encoding, EyreWidth width, int widths) {
	writeVarLengthInt(getOpcode(encoding, width, widths));
}



static inline void writeOpReg(int opcode, int offset) {
	int length = (8 + (_bit_scan_reverse(opcode | 1) & -8)) >> 3;
	writeVarLengthInt(opcode + (offset << ((length - 1) << 2)));
}



static inline void checkPrefix(EyreEncoding* encoding) {
	if(encoding->prefix != 0) write8(encoding->prefix);
}



static void addRelocation(char width, void* node, int offset) {
	Relocation* relocation = &relocations[relocationCount++];
	relocation->pos = bufferPos;
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



static int relocAndDisp(int mod, int disp, void* node) {
	if(hasMemReloc) {
		addDefaultRelocation(WIDTH_DWORD, node);
		write32(0);
	} else if(mod == 1) {
		write8(disp);
	} else if(mod == 2) {
		write32(disp);
	}
}



static int writeMem(
	int opcode,
	void* node,
	int rexW,
	int rexR,
	int reg,
	int immLength
) {
	int disp  = resolveMem(node);
	int base  = baseReg;
	int index = indexReg;
	int scale = indexScale;

	if(aso == 1) write8(0x67);

	int mod;

	if(hasMemReloc) {
		mod = 2; // disp32
	} else if(disp == 0) {
		if(base == 5)
			mod = 1; // disp8, rbp as base needs an offset
		else
			mod = 0; // no disp
	} else if(isImm8(disp)) {
		mod = 1; // disp8
	} else {
		mod = 2; // disp32
	}

	if(index >= 0) { // SIB
		int scaleValue = _bit_scan_forward(scale);

		if(base >= 0) {
			writeRex(rexW, rexR, regRex(index), regRex(base));
			writeVarLengthInt(opcode);
			writeModRM(mod, reg, 0b100);
			writeSib(scaleValue, index, base);
			relocAndDisp(mod, disp, node);
		} else {
			writeRex(rexW, rexR, regRex(index), 0);
			writeVarLengthInt(opcode);
			writeModRM(0, reg, 0b100);
			writeSib(scaleValue, index, 0b101);
			relocAndDisp(mod, disp, node);
		}
	} else if(base >= 0) { // Indirect
		writeRex(rexW, rexR, 0, regRex(base));
		writeVarLengthInt(opcode);

		if(isSP(base)) {
			writeModRM(mod, reg, 0b100);
			writeSib(0, 0b100, 0b100);
		} else {
			writeModRM(mod, reg, base & 7);
		}

		relocAndDisp(mod, disp, node);
	} else if(isOdd(memRelocCount)) { // RIP-relative
		writeRex(rexW, rexR, 0, 0);
		writeVarLengthInt(opcode);
		writeModRM(0b00, reg, 0b101);
		addRelativeRelocation(WIDTH_DWORD, node, immLength);
		write32(0);
	} else if(mod != 0) { // Absolute 32-bit
		writeRex(rexW, rexR, 0, 0);
		writeVarLengthInt(opcode);
		writeModRM(0b00, reg, 0b100);
		writeSib(0b00, 0b100, 0b101);
		relocAndDisp(mod, disp, node);
	} else { // Empty memory operand
		invalidEncoding();
	}
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



// Base encoding



static void encodeNoneRaw(EyreOperands operands) {
	EyreEncoding* encoding = getEncoding(operands);
	checkPrefix(encoding);
	writeVarLengthInt(encoding->opcode);
}



static void encodeNone(EyreOperands operands, int width) {
	EyreEncoding* encoding = getEncoding(operands);
	checkPrefix(encoding);
	checkWidths(width, encoding->widths);
	checkO16(width);
	writeRex(rexw(width, encoding->widths), 0, 0, 0);
	writeOpcode(encoding, width, encoding->widths);
}



static void encode1R(EyreOperands operands, RegNode* op1) {
	EyreEncoding* encoding = getEncoding(operands);
	int widths = encoding->widths;
	int width = op1->width;
	int value = op1->value;
	checkWidths(width, widths);
	checkO16(width);
	checkPrefix(encoding);
	writeRex(rexw(width, widths), 0, 0, regRex(value));
	writeOpcode(encoding, width, widths);
	writeModRM(0b11, encoding->extension, value & 7);
}



static void encode2RR(EyreOperands operands, RegNode* op1, RegNode* op2) {
	EyreEncoding* encoding = getEncoding(operands);
	int widths = encoding->widths;
	int width = op1->width;
	checkWidths(width, widths);
	checkO16(width);
	checkPrefix(encoding);
	writeRex(rexw(width, widths), regRex(op2->value), 0, regRex(op1->value));
	writeOpcode(encoding, width, widths);
	writeModRM(0b11, op2->value, op1->value & 7);
}



static void encode2RM(EyreOperands operands, RegNode* op1, MemNode* op2, int immLength) {
	EyreEncoding* encoding = getEncoding(operands);
	int widths = encoding->widths;
	int width = op1->width;
	checkWidths(width, widths);
	checkO16(width);
	checkPrefix(encoding);
	writeMem(
		getOpcode(encoding, width, widths),
		op2->value,
		rexw(width, widths),
		regRex(op1->value),
		op1->value,
		immLength
	);
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
	writeOpReg(getOpcode(encoding, width, widths), value & 7);
}



static void encode1M(EyreOperands operands, MemNode* op1, int immLength) {
	EyreEncoding* encoding = getEncoding(operands);
	int widths = encoding->widths;
	int width = op1->width;
	if(width < 0) invalidEncoding();
	checkWidths(width, widths);
	checkO16(width);
	writeMem(
		getOpcode(encoding, width, widths),
		op1->value,
		rexw(width, widths),
		0,
		encoding->extension,
		immLength
	);
}



// Instruction assembly



static void assemble0() {
	encodeNoneRaw(OPERANDS_NONE);
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
		encode1M(OPERANDS_M, node->op1, 0);
	} else {
		assemblerError("Invalid encoding");
	}
}



static void assemble2R(InsNode* node) {
	RegNode* op1 = node->op1;
	void* op2 = node->op2;
	int width = op1->width;

	if(nodeType(op2) == NODE_REG) {
		int r2 = ((RegNode*) op2)->value;
		int width2 = ((RegNode*) op2)->width;

		if(hasSpecifier(SPECIFIER_RM_CL) && r2 == 1 && width2 == WIDTH_BYTE) {
			encode1R(OPERANDS_RM_CL, op1);
		} else if(width != width2) {
			invalidEncoding();
		} else {
			encode2RR(OPERANDS_R_R, op1, op2);
		}
	} else if(nodeType(op2) == NODE_MEM) {
		int width2 = ((MemNode*) op2)->width;
		if(width2 >= 0 && width2 != width)
			invalidEncoding();
		encode2RM(OPERANDS_R_M, op1, op2, 0);
	} else if(nodeType(op2) == NODE_IMM) {
		int imm = resolveImm(op2);

		if(hasSpecifier(SPECIFIER_RM_I8) && !hasImmReloc && width != WIDTH_BYTE && isImm8(imm)) {
			encode1R(OPERANDS_R_I8, op1);
			writeImm(op2, WIDTH_BYTE, imm);
		} else if(hasSpecifier(SPECIFIER_RM_1) && !hasImmReloc && imm == 1) {
			encode1R(OPERANDS_RM_1, op1);
		} else if(hasSpecifier(SPECIFIER_A_I) && op1->value == 0) {
			encodeNone(OPERANDS_A_I, op1->width);
			writeImm(op2, width, imm);
		} else {
			encode1R(OPERANDS_R_I, op1);
			writeImm(op2, width, imm);
		}
	} else {
		invalidEncoding();
	}
}



static void assemble2M(InsNode* node) {
	MemNode* op1 = node->op1;
	void* op2 = node->op2;

	if(nodeType(op2) == NODE_REG) {
		int r2 = ((RegNode*) op2)->value;
		int width2 = ((RegNode*) op2)->width;

		if(hasSpecifier(SPECIFIER_RM_CL) && r2 == 1 && width2 == WIDTH_BYTE) {
			encode1M(OPERANDS_RM_CL, op1, 0);
		} else {
			if(op1->width >= 0 && op1->width != width2)
				invalidEncoding();
			encode2RM(OPERANDS_M_R, op2, op1, 0);
		}
	} else if(nodeType(op2) == NODE_IMM) {
		int width = op1->width;
		if(width < 0) invalidEncoding();
		int imm = resolveImm(op2);

		if(hasSpecifier(SPECIFIER_RM_I8) && !hasImmReloc && width != WIDTH_BYTE && isImm8(imm)) {
			encode1M(OPERANDS_M_I8, op1, 1);
			writeImm(op2, WIDTH_BYTE, imm);
		} else if(hasSpecifier(SPECIFIER_RM_1) && !hasImmReloc && imm == 1) {
			encode1M(OPERANDS_RM_1, op1, 0);
		} else {
			encode1M(OPERANDS_M_I, op1, width == WIDTH_QWORD ? 4 : 1 << width);
			writeImm(op2, width, imm);
		}
	} else {
		invalidEncoding();
	}
}



static void assembleInstruction_(InsNode* node) {
	if(node->op1 == NULL)
		assemble0();
	else if(node->op2 == NULL)
		assemble1(node);
	else if(node->op3 == NULL)
		if(nodeType(node->op1) == NODE_REG)
			assemble2R(node);
		else if(nodeType(node->op1) == NODE_MEM)
			assemble2M(node);
		else
			invalidEncoding();
	else if(node->op4 == NULL)
		assemblerError("3-operand instructions not yet supported");
	else
		assemblerError("4-operand instructions not yet supported");
}



static void assembleInstruction(InsNode* node) {
	group = eyreGetEncodings(node->mnemonic);

	CustomEncoding customEncoding = customEncodings[node->mnemonic];

	if(customEncoding != NULL) {
		customEncoding(node);
		return;
	}

	assembleInstruction_(node);
}



// Other nodes



static void handleLabel(LabelNode* node) {
	node->symbol->pos = bufferPos;
	if(node->symbol->base.name == EYRE_INTERN_MAIN) {
		if(entryPoint != NULL)
			assemblerError("Redeclaration of entry point");
		entryPoint = (PosSymbol*) node->symbol;
	}
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



static void customEncodeJECXZ(InsNode* node) {
	if(node->size != 1) invalidEncoding();
	int imm = resolveImm(node->op1);

	if(hasImmReloc) {
		encodeNoneRaw(OPERANDS_CUSTOM1);
		addRelativeRelocation(WIDTH_BYTE, node->op1, 0);
		write8(0);
	} else {
		encodeNoneRaw(OPERANDS_CUSTOM1);
		write8(imm);
	}
}



static void customEncodeJCC(InsNode* node) {
	if(node->size != 1) invalidEncoding();
	int imm = resolveImm(node->op1);

	if(hasImmReloc) {
		encodeNoneRaw(OPERANDS_CUSTOM2);
		addRelativeRelocation(WIDTH_DWORD, node->op1, 0);
		write32(0);
	} else if(isImm8(imm)) {
		encodeNoneRaw(OPERANDS_CUSTOM1);
		write8(imm);
	} else {
		encodeNoneRaw(OPERANDS_CUSTOM2);
		write32(imm);
	}
}



static void customEncodeCALL(InsNode* node) {
	if(node->op2 != NULL)
		invalidEncoding();
	int imm = resolveImm(node->op1);
	encodeNoneRaw(OPERANDS_CUSTOM1);
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
		encodeNoneRaw(OPERANDS_NONE);
		return;
	}

	if(node->op2 != NULL || nodeType(node->op1) != NODE_IMM)
		invalidEncoding();

	int imm = resolveImm(node->op1);
	encodeNoneRaw(OPERANDS_CUSTOM1);
	if(hasImmReloc) addDefaultRelocation(WIDTH_WORD, node);
	if(!isImm16(imm)) assemblerError("Invalid encoding");
	write16(imm);
}



static void customEncodePUSH(InsNode* node) {
	if(node->op2 != NULL) assemblerError("Invalid encoding");

	if(nodeType(node->op1) != NODE_IMM) {
		assembleInstruction_(node);
		return;
	}

	int imm = resolveImm(node->op1);

	if(hasImmReloc) {
		write8(0x68);
		addDefaultRelocation(WIDTH_DWORD, node->op1);
		write32(0);
	} else if(isImm8(imm)) {
		write8(0x6A);
		write8(imm);
	} else if(isImm16(imm)) {
		write16(0x6866);
		write16(imm);
	} else {
		write8(0x68);
		write32(imm);
	}
}



static void customEncodeIMUL(InsNode* node) {
	if(node->op2 == NULL || node->op3 == NULL)
		assembleInstruction_(node);
	if(node->op4 != NULL)
		invalidEncoding();
	assemblerError("3-operand IMUL is not yet supported");
}



static void customEncodeMOVSX(InsNode* node) {
	if(node->op3 != NULL) invalidEncoding();
	if(nodeType(node->op1) != NODE_REG) invalidEncoding();

	int width = ((RegNode*) node->op1)->width;

	if(nodeType(node->op2) == NODE_REG) {
		int width2 = ((RegNode*) node->op2)->width;

		if(width2 == WIDTH_BYTE) {
			encode2RR(OPERANDS_CUSTOM1, node->op1, node->op2);
		} else if(width2 == WIDTH_WORD) {
			encode2RR(OPERANDS_CUSTOM2, node->op1, node->op2);
		} else {
			invalidEncoding();
		}
	} else if(nodeType(node->op2) == NODE_MEM) {
		int width2 = ((MemNode*) node->op2)->width;

		if(width2 == WIDTH_BYTE) {
			encode2RM(OPERANDS_CUSTOM1, node->op1, node->op2, 0);
		} else if(width2 == WIDTH_WORD) {
			encode2RM(OPERANDS_CUSTOM2, node->op1, node->op2, 0);
		} else {
			invalidEncoding();
		}
	}
}



static void customEncodeMOVSXD(InsNode* node) {
	if(node->op3 != NULL || nodeType(node->op1) != NODE_REG) invalidEncoding();

	int width = ((RegNode*) node->op1)->width;

	if(nodeType(node->op2) == NODE_REG) {
		int width2 = ((RegNode*) node->op2)->width;

		if(width != WIDTH_QWORD || width2 != WIDTH_DWORD)
			invalidEncoding();

		encode2RR(OPERANDS_CUSTOM1, node->op1, node->op2);
	} else if(nodeType(node->op2) == NODE_MEM) {
		int width2 = ((MemNode*) node->op2)->width;

		if(width != WIDTH_QWORD || width2 != WIDTH_DWORD)
			invalidEncoding();

		encode2RM(OPERANDS_CUSTOM1, node->op1, node->op2, 0);
	} else {
		invalidEncoding();
	}
}



static void customEncodeXCHG(InsNode* node) {
	if(node->op3 != NULL)
		invalidEncoding();
	if(nodeType(node->op1) != NODE_REG || nodeType(node->op2) != NODE_REG)
		assembleInstruction_(node);
	else if(((RegNode*) node->op1)->value == 0)
		encode1O(OPERANDS_CUSTOM1, node->op2);
	else if(((RegNode*) node->op2)->value == 0)
		encode1O(OPERANDS_CUSTOM2, node->op1);
	else
		assembleInstruction_(node);
}



static inline int isReg(void* node) {
	return nodeType(node) == NODE_REG;
}

static inline int isMem(void* node) {
	return nodeType(node) == NODE_MEM;
}

static inline int isImm(void* node) {
	return nodeType(node) == NODE_IMM;
}

static inline int regValue(RegNode* node) {
	return node->value;
}

static inline int regWidth(RegNode* node) {
	return node->width;
}



static void customEncodeIN(InsNode* node) {
	if(node->size != 2 || !isReg(node->op1))
		invalidEncoding();
	if(regValue(node->op1) != 0)
		invalidEncoding();

	int width = regWidth(node->op1);

	if(isReg(node->op2)) {
		if(regValue(node->op2) != 2 || regWidth(node->op2) != WIDTH_WORD)
			invalidEncoding();
		encodeNone(OPERANDS_CUSTOM2, width);
	} else if(isImm(node->op2)) {
		int imm = resolveImm(node->op2);
		encodeNone(OPERANDS_CUSTOM1, width);
		writeImm(node->op2, WIDTH_BYTE, imm);
	}
}



static void customEncodeOUT(InsNode* node) {
	if(node->size != 2 || !isReg(node->op2))
		invalidEncoding();
	if(regValue(node->op2) != 0)
		invalidEncoding();

	int width = regWidth(node->op2);

	if(isReg(node->op1)) {
		if(regValue(node->op1) != 2 || regWidth(node->op1) != WIDTH_WORD)
			invalidEncoding();
		encodeNone(OPERANDS_CUSTOM2, width);
	} else if(isImm(node->op1)) {
		int imm = resolveImm(node->op1);
		encodeNone(OPERANDS_CUSTOM1, width);
		writeImm(node->op1, WIDTH_BYTE, imm);
	}
}



static CustomEncoding customEncodings[MNEMONIC_COUNT] = {
	[MNEMONIC_CALL]   = customEncodeCALL,
	[MNEMONIC_JMP]    = customEncodeJCC,
	[MNEMONIC_JA]     = customEncodeJCC,
	[MNEMONIC_JAE]    = customEncodeJCC,
	[MNEMONIC_JB]     = customEncodeJCC,
	[MNEMONIC_JBE]    = customEncodeJCC,
	[MNEMONIC_JC]     = customEncodeJCC,
	[MNEMONIC_JE]     = customEncodeJCC,
	[MNEMONIC_JG]     = customEncodeJCC,
	[MNEMONIC_JGE]    = customEncodeJCC,
	[MNEMONIC_JL]     = customEncodeJCC,
	[MNEMONIC_JLE]    = customEncodeJCC,
	[MNEMONIC_JNA]    = customEncodeJCC,
	[MNEMONIC_JNAE]   = customEncodeJCC,
	[MNEMONIC_JNB]    = customEncodeJCC,
	[MNEMONIC_JNBE]   = customEncodeJCC,
	[MNEMONIC_JNC]    = customEncodeJCC,
	[MNEMONIC_JNE]    = customEncodeJCC,
	[MNEMONIC_JNG]    = customEncodeJCC,
	[MNEMONIC_JNGE]   = customEncodeJCC,
	[MNEMONIC_JNL]    = customEncodeJCC,
	[MNEMONIC_JNLE]   = customEncodeJCC,
	[MNEMONIC_JO]     = customEncodeJCC,
	[MNEMONIC_JP]     = customEncodeJCC,
	[MNEMONIC_JPE]    = customEncodeJCC,
	[MNEMONIC_JPO]    = customEncodeJCC,
	[MNEMONIC_JS]     = customEncodeJCC,
	[MNEMONIC_JZ]     = customEncodeJCC,
	[MNEMONIC_JECXZ]  = customEncodeJECXZ,
	[MNEMONIC_JRCXZ]  = customEncodeJECXZ,
	[MNEMONIC_INT]    = customEncodeINT,
	[MNEMONIC_RET]    = customEncodeRET,
	[MNEMONIC_RETF]   = customEncodeRET,
	[MNEMONIC_PUSH]   = customEncodePUSH,
	[MNEMONIC_MOVSX]  = customEncodeMOVSX,
	[MNEMONIC_MOVZX]  = customEncodeMOVSX,
	[MNEMONIC_MOVSXD] = customEncodeMOVSXD,
	[MNEMONIC_IMUL]   = customEncodeIMUL,
	[MNEMONIC_XCHG]   = customEncodeXCHG,
	[MNEMONIC_IN]     = customEncodeIN,
	[MNEMONIC_OUT]    = customEncodeOUT
};



// Public functions



void* getAssemblerBuffer() {
	return buffer;
}



int getAssemblerBufferLength() {
	return bufferPos;
}