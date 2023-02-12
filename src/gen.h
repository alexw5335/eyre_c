#ifndef INCLUDE_GEN
#define INCLUDE_GEN



#include "enums.h"



typedef struct {
	char* mnemonic;
	int opcode;
	int extension;
	EyreOperands operands;
	int widths;
} EyreGenEncoding;



typedef struct {
	char mnemonic[16];
	int operandsBits;
	int specifierBits;
	int encodingCount;
	EyreGenEncoding* encodings[32];
} EyreGenGroup;



void eyreGen(char* path);


#endif