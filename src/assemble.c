#include "assemble.h"
#include "log.h"
#include "eyre.h"
#include "parse.h"
#include <intrin.h>
#include "intern.h"


static int isImm8(s64 input) {
	return input >= INT8_MIN && input <= INT8_MAX;
}

static int isImm16(s64 input) {
	return input >= INT16_MIN && input <= INT16_MAX;
}

static int isImm32(s64 input) {
	return input >= INT32_MIN && input <= INT32_MAX;
}


#define bufferCapacity 66536
static char buffer[bufferCapacity + 8];
static void* bufferPos = buffer;
static void* bufferEnd = buffer + bufferCapacity;
static SrcFile* srcFile;
static int pos;
static EyreGroup* group;



typedef struct {
	
} Relocation;



#define assemblerErrorOffset(format, offset, ...) assemblerError_(format, offset, __FILE__, __LINE__, ##__VA_ARGS__)

#define assemblerError(format, ...) assemblerError_(format, 1, __FILE__, __LINE__, ##__VA_ARGS__)

static void assemblerError_(char* format, int offset, char* file, int line, ...) {
	if(pos - offset >= srcFile->nodeCount)
		eyreError("Invalid assembler error node index: %d", pos - offset);
	fprintf(stdout, "Assembler error at %s:%d: ", srcFile->path, srcFile->nodeLines[pos - offset]);
	va_list args;
	va_start(args, line);
	vfprintf(stdout, format, args);
	fprintf(stdout, "\n");
	eyreError_("Assembler error", file, line);
}



static inline void checkBuffer() {
	if(bufferPos >= bufferEnd)
		assemblerError("Capacity reached");
}



static void write8(char value) {
	*(char*) bufferPos = value;
	bufferPos++;
	checkBuffer();
}

static void write16(short value) {
	*(short*) bufferPos = value;
	bufferPos += 2;
	checkBuffer();
}

static void write32(int value) {
	*(int*) bufferPos = value;
	bufferPos += 4;
	checkBuffer();
}

static void writeInt(int value) {
	*(int*) bufferPos = value;
	bufferPos += (7 + _bit_scan_reverse(value | 1) & -8) >> 3;
	checkBuffer();
}



static int resolveImmRec(void* n) {
	char type = *(char*) n;

	if(type == NODE_INT) {
		IntNode* node = n;
		return node->value;
	}

	return 0;
}



static EyreEncoding* getEncoding(EyreOperands operands) {
	if(group->operandsBits && (1 << operands) == 0)
		assemblerError("Invalid operands");
	int index = _popcnt32(group->operandsBits & ((1 << operands) - 1));
	return &group->encodings[index];
}



static int hasSpecifier(EyreSpecifier specifier) {
	return (group->specifierBits & (1 << specifier)) != 0;
}



static void encodeNone(EyreOperands operands) {
	EyreEncoding* encoding = getEncoding(operands);
	if(encoding->prefix != 0) write8(encoding->prefix);
	writeInt(encoding->opcode);
}

static void assemble0() {
	encodeNone(OPERANDS_NONE);
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

static void writeRexChecked(int w, int r, int x, int b, int required, int disallowed) {
	int value = 0b01000000 | (w << 3) | (r << 2) | (x << 1) | b;

	if(value != 0b01000000)
		if(disallowed)
			assemblerError("Invalid encoding");
		else
			write8(value);
	else
		assemblerError("Invalid encoding");
}

static inline int rexw(int width, int widths) {
	return (widths >> 2) & (width == WIDTH_QWORD);
}

static inline int checkWidths(int width, int widths) {
	if(((1 << width) & widths) == 0)
		assemblerError("Invalid encoding");
}

static inline int checkO16(int width) {
	if(width == WIDTH_WORD) write8(0x66);
}

static void writeOpcode(EyreEncoding* encoding, EyreWidth width, int widths) {
	writeInt(encoding->opcode + ((width != WIDTH_BYTE) & (widths >> 2)));
}

static inline void writeOpReg(int opcode, int offset) {
	int length = (7 + _bit_scan_reverse(opcode | 1) & -8) >> 3;
	writeInt(opcode + (offset << ((length - 1) << 2)));
}

static void encode1R(EyreOperands operands, RegNode* op1) {
	EyreEncoding* encoding = getEncoding(operands);
	int widths = encoding->widths;
	int width = op1->width;
	int value = op1->value;
	checkWidths(width, widths);
	checkO16(width);
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
	writeRex(rexw(width, widths), 0, 0, (value >> 3) & 1);
	writeOpReg(encoding->opcode, value & 7);
}

static void assemble1(InsNode* node) {
	int type1 = node->op1->type;

	if(type1 == NODE_REG) {
		if(hasSpecifier(SPECIFIER_O)) {
			encode1O(OPERANDS_O, (RegNode*) node->op1);
		} else {
			encode1R(OPERANDS_R, (RegNode*) node->op1);
		}
	}
}

static void assemble2(InsNode* node) {

}

static void assembleInstruction(InsNode* node) {
	group = eyreGetEncodings(node->mnemonic);

	if(node->op1 == NULL)
		assemble0();
	else if(node->op2 == NULL)
		assemble1(node);
	else if(node->op3 == NULL)
		assemble2(node);
	else if(node->op4 == NULL)
		assemblerError("3-operand instructions not yet supported");
	else
		assemblerError("4-operand instructions not yet supported");
}



static void handleLabel(LabelNode* node) {
	node->symbol->pos = (int) (bufferPos - (void*) buffer);
}



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



void* eyreGetAssemblerBuffer() {
	return buffer;
}



int eyreGetAssemblerBufferSize() {
	return (int) (bufferPos - (void*) buffer);
}