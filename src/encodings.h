#ifndef INCLUDE_ENCODINGS
#define INCLUDE_ENCODINGS



#include "enums.h"



typedef struct {
	int  opcode;
	char extension;
	char prefix;
	char widths;
} EyreEncoding;



typedef struct {
	int operandsBits;
	int specifierBits;
	EyreEncoding* encodings;
} EyreGroup;



EyreGroup* eyreGetEncodings(int mnemonic);



#endif