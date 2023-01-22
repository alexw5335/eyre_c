#pragma once

/*
	R,
	M,
	O(Specifier.O),
	I8(Specifier.I8),
	I16(Specifier.I16),
	I32(Specifier.I32),
	REL8(Specifier.REL8),
	REL32(Specifier.REL32),

	R_R,
	R_M,
	M_R,
	R_I,
	R_I8(Specifier.RM_I8),
	M_I,
	M_I8(Specifier.RM_I8),
	A_I(Specifier.A_I),
	RM_1(Specifier.RM_1),
	RM_CL(Specifier.RM_CL),

	CUSTOM1,
	CUSTOM2;
*/

typedef enum {
	OPERANDS_NONE,
	OPERANDS_R,
	OPERANDS_M,
	OPERANDS_O,
	OPERANDS_I8,
	OPERANDS_I16,
	OPERANDS_I32,
	OPERANDS_REL8,
	OPERANDS_REL32,
	OPERANDS_R_R,
	OPERANDS_R_M,
	OPERANDS_M_R,
	OPERANDS_R_I,
	OPERANDS_M_I,
	OPERANDS_R_I8,
	OPERANDS_M_I8,
	OPERANDS_A_I,
	OPERANDS_RM_1,
	OPERANDS_RM_CL,
	OPERANDS_CUSTOM1,
	OPERANDS_CUSTOM2,
	OPERANDS_COUNT,
} Operands;



static char* operandsNames[OPERANDS_COUNT] = {
	"NONE","R","M","O",
	"I8","I16","I32","REL8",
	"REL32","R_R","R_M","M_R",
	"R_I","M_I","R_I8","M_I8",
	"A_I","RM_1","RM_CL","CUSTOM1",
	"CUSTOM2"
};



typedef enum {
	COMPOUND_RM,
	COMPOUND_RM_R,
	COMPOUND_R_RM,
	COMPOUND_RM_I,
	COMPOUND_RM_I8,
	COMPOUND_COUNT,
} Compound;



static char* compoundNames[COMPOUND_COUNT] = {
	"RM", "RM_R", "R_RM", "RM_I", "RM_I8"
};



static Operands compoundMap[COMPOUND_COUNT][5] = {
	{ OPERANDS_R, OPERANDS_M },
	{ OPERANDS_R_R, OPERANDS_M_R },
	{ OPERANDS_R_R, OPERANDS_R_M },
	{ OPERANDS_R_I, OPERANDS_M_I },
	{ OPERANDS_R_I8, OPERANDS_M_I8 }
};



typedef struct {
	char chars[16];
} Mnemonic;



typedef struct {
	int      mnemonic;
	int      opcode;
	int      extension;
	Operands operands;
	int      widths;
} Encoding;



typedef struct {
	int mnemonic;
	int operandsBits;
	int specificBits;
	int encodings[32];
	int encodingCount;
} EncodingGroup;