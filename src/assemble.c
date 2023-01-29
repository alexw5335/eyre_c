#include "assemble.h"
#include "log.h"
#include "eyre.h"
#include "parse.h"



#define bufferCapacity 66536
static char buffer[bufferCapacity + 8];
static void* pos = buffer;
static void* end = buffer + bufferCapacity;



static inline void checkBuffer() {
	if(pos >= end)eyreError("Capacity reached");
}



static void i8(char value) {
	*(char*) pos = value;
	checkBuffer();
}

static void i16(short value) {
	*(short*) pos = value;
	checkBuffer();
}

static void i32(int value) {
	*(int*) pos = value;
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



static void assembleInstruction(InsNode* node) {

}



static void handleLabel(LabelNode* node) {
	node->symbol->pos = (int) (pos - (void*) buffer);
}



void eyreAssemble(SrcFile* srcFile) {
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