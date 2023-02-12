#ifndef INCLUDE_ENCODINGS
#define INCLUDE_ENCODINGS



#include "enums.h"



typedef struct {
	int opcode;
	char extension;
	char prefix;
	char widths;
} EyreEncoding;



typedef struct {
	int operandsBits;
	int specifierBits;
	EyreEncoding* encodings;
} EyreGroup;



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



EyreGroup* eyreGetEncodings(int mnemonic);



#endif