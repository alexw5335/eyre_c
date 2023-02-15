#include "enums.h"


char* eyreNodeNames[NODE_COUNT] = {
	"REG", "INT", "SYM", "BINARY",
	"UNARY", "MEM", "IMM", "INS",
	"STRUCT", "SCOPE_END", "NAMESPACE", "ENUM",
	"LABEL", "DOT", "INVOKE", "REL",
};

char* eyreUnaryOpSymbols[UNARY_COUNT] = {
	"+", "-", "~"
};

char* eyreBinaryOpSymbols[BINARY_COUNT] = {
	"+", "-", "*", "/", "&", "|", "^", "<<", ">>", "", ""
};

char* eyreByteRegNames[16] = {
	"al","cl","dl","bl","ah","ch","dh","bh",
	"r8b","r9b","r10b","r11b","r12b","r13b","r14b","r15b",
};

char* eyreWordRegNames[16] = {
	"ax","cx","dx","bx","sp","bp","si","di",
	"r8w","r9w","r10w","r11w","r12w","r13w","r14w","r15w",
};

char* eyreDWordRegNames[16] = {
	"eax","ecx","edx","ebx","esp","ebp","esi","edi",
	"r8d","r9d","r10d","r11d","r12d","r13d","r14d","r15d",
};

char* eyreQWordRegNames[16] = {
	"rax","rcx","rdx","rbx","rsp","rbp","rsi","rdi",
	"r8","r9","r10","r11","r12","r13","r14","r15",
};

char* eyreKeywordNames[KEYWORD_COUNT] = {
	"const", "struct", "namespace", "proc",
	"enum", "bitmask", "dllimport", "var"
};

char* eyreWidthNames[WIDTH_COUNT] = {
	"byte", "word", "dword", "qword"
};

char* eyreVarWidthNames[VARWIDTH_COUNT] = {
	"db", "dw", "dd", "dq",
};

char* eyreSpecifierNames[] = {
	"NONE", "O", "RM_I8", "RM_1",
	"RM_CL"
};

char* eyreCompoundOperandsNames[COMPOUND_COUNT] = {
	"RM", "RM_R", "R_RM", "RM_I", "RM_I8"
};



EyreOperands eyreCompoundOperandsMap[COMPOUND_COUNT][3] = {
	{ OPERANDS_R, OPERANDS_M },
	{ OPERANDS_R_R, OPERANDS_M_R },
	{ OPERANDS_R_R, OPERANDS_R_M },
	{ OPERANDS_R_I, OPERANDS_M_I },
	{ OPERANDS_R_I8, OPERANDS_M_I8 }
};



char* eyreOperandsNames[OPERANDS_COUNT] = {
	"NONE","R","M","O",
	"R_R","R_M","M_R","R_I",
	"M_I","R_I8","M_I8","A_I",
	"RM_1","RM_CL","CUSTOM1","CUSTOM2"
};



int eyreOperandsSpecifiers[OPERANDS_COUNT] = {
	[OPERANDS_O]     = SPECIFIER_O,
	[OPERANDS_RM_1]  = SPECIFIER_RM_1,
	[OPERANDS_RM_CL] = SPECIFIER_RM_CL,
	[OPERANDS_R_I8]  = SPECIFIER_RM_I8,
	[OPERANDS_M_I8]  = SPECIFIER_RM_I8,
	[OPERANDS_A_I]   = SPECIFIER_A_I
};



char* eyreTokenSymbols[TOKEN_COUNT] = {
	"","","","","","",
	"+","-","=","==",
	"!=","!","(",")",
	"[","]","{","}",
	";",":","<","<=",
	">",">=",".","/",
	"~","|","&","*",
	"::","&&","||",",",
	"<<", ">>", "^",
};



char* eyreTokenNames[TOKEN_COUNT] = {
	"END","INT","LONG","CHAR","STRING","ID",
	"PLUS","MINUS","EQUALS","EQUALITY",
	"INEQUALITY","EXCLAMATION","LPAREN","RPAREN",
	"LBRACKET","RBRACKET","LBRACE","RBRACE",
	"SEMICOLON","COLON","LT","LTE",
	"GT","GTE","DOT","SLASH",
	"TILDE","PIPE","AMPERSAND","ASTERISK",
	"REFERENCE","LOGICAL_AND","LOGICAL_OR","COMMA",
	"LSHIFT", "RSHIFT", "CARET",
};



int calcUnaryInt(EyreUnaryOp op, int value) {
	switch(op) {
		case UNARY_POS: return value;
		case UNARY_NEG: return -value;
		case UNARY_NOT: return ~value;
		default: return value;
	}
}



int calcBinaryInt(EyreBinaryOp op, int left, int right) {
	switch(op) {
		case BINARY_ADD: return left + right;
		case BINARY_SUB: return left - right;
		case BINARY_MUL: return left * right;
		case BINARY_DIV: return left / right;
		case BINARY_AND: return left & right;
		case BINARY_OR:  return left | right;
		case BINARY_XOR: return left ^ right;
		case BINARY_SHL: return left << right;
		case BINARY_SHR: return left >> right;
		case BINARY_DOT: return 0;
		case BINARY_INV: return 0;
		default: return 0;
	}
}