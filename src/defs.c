#include "defs.h"
char* eyreNodeNames[NODE_COUNT] = {
	"REG", "INT", "SYM", "BINARY",
	"UNARY", "MEM", "IMM", "INS",
	"STRUCT", "SCOPE_END", "NAMESPACE", "ENUM"
};

char* eyreUnaryOpSymbols[UNARY_COUNT] = {
	"+", "-", "~"
};

char* eyreBinaryOpSymbols[BINARY_COUNT] = {
	"+", "-", "*", "/", "&", "|", "^", "<<", ">>"
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
	"enum", "bitmask",
};
char* eyreWidthNames[WIDTH_COUNT] = {
	"byte", "word", "dword", "qword"
};

char* eyreSpecifierNames[] = {
	"NONE", "O", "I8", "I32",
	"REL8", "REL32", "RM_I8", "RM_1",
	"RM_CL"
};

char* eyreMnemonicNames[MNEMONIC_COUNT] = {
	"add8", "or8", "adc8", "sbb8",
	"and8", "sub8", "xor8", "cmp8",
	"jmp8", "add", "or", "adc",
	"sbb", "and", "sub", "xor",
	"cmp", "push", "push_fs", "push_gs",
	"pushw_fs", "pushw_gs", "pop", "pop_fs",
	"pop_gs", "popw_fs", "popw_gs", "movsxd",
	"movsx", "movzx", "insb", "insw",
	"insd", "outsb", "outsw", "outsd",
	"ja", "jae", "jb", "jbe",
	"jc", "je", "jg", "jge",
	"jl", "jle", "jna", "jnae",
	"jnb", "jnbe", "jnc", "jne",
	"jng", "jnge", "jnl", "jnle",
	"jno", "jnp", "jns", "jnz",
	"jo", "jp", "jpe", "jpo",
	"js", "jz", "test", "xchg",
	"mov", "lea", "nop", "cbw",
	"cwde", "cdqe", "cwd", "cdq",
	"cqo", "wait", "fwait", "pushf",
	"pushfq", "lahf", "movsb", "movsw",
	"movsd", "movsq", "cmpsb", "cmpsw",
	"cmpsd", "cmpsq", "stosb", "stosw",
	"stosd", "stosq", "scasb", "scasw",
	"scasd", "scasq", "lodsb", "lodsw",
	"lodsd", "lodsq", "rol", "ror",
	"rcl", "rcr", "sal", "shl",
	"shr", "sar", "ret", "retf",
	"leavew", "leave", "int3", "int",
	"int1", "iretw", "iretd", "iretq",
	"jcxz", "jrcxz", "in", "out",
	"hlt", "cmc", "not", "neg",
	"mul", "imul", "div", "idiv",
	"clc", "stc", "cli", "sti",
	"cld", "std", "inc", "dec",
	"call", "callf", "jmp", "jmpf",
	"rdrand", "rdseed",
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
	"I8","I16","I32","REL8",
	"REL32","R_R","R_M","M_R",
	"R_I","M_I","R_I8","M_I8",
	"A_I","RM_1","RM_CL","CUSTOM1",
	"CUSTOM2"
};



int eyreOperandsSpecifiers[OPERANDS_COUNT] = {
	SPECIFIER_NONE,
	SPECIFIER_NONE,
	SPECIFIER_NONE,
	SPECIFIER_O,
	SPECIFIER_I8,
	SPECIFIER_I16,
	SPECIFIER_I32,
	SPECIFIER_REL8,
	SPECIFIER_REL32,
	SPECIFIER_NONE,
	SPECIFIER_NONE,
	SPECIFIER_NONE,
	SPECIFIER_NONE,
	SPECIFIER_NONE,
	SPECIFIER_RM_I8,
	SPECIFIER_RM_I8,
	SPECIFIER_NONE,
	SPECIFIER_RM_1,
	SPECIFIER_RM_CL,
	SPECIFIER_NONE,
	SPECIFIER_NONE,
};