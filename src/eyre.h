#ifndef EYRE_INCLUDE
#define EYRE_INCLUDE



#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>



// Types



#define TRUE 1
#define FALSE 0

typedef unsigned char      u8;
typedef signed char        s8;
typedef unsigned short     u16;
typedef signed short       s16;
typedef unsigned int       u32;
typedef signed int         s32;
typedef unsigned long long u64;
typedef signed long long   s64;



// Enums



typedef enum EyreNodeType {
	NODE_REG,
	NODE_INT,
	NODE_SYM,
	NODE_BINARY,
	NODE_UNARY,
	NODE_MEM,
	NODE_IMM,
	NODE_INS,
	NODE_COUNT,
} EyreNodeType;



static char* eyreNodeNames[NODE_COUNT] = {
	"REG", "INT", "SYM", "BINARY",
	"UNARY", "MEM", "IMM", "INS",
};



typedef enum EyreUnaryOp {
	UNARY_POS,
	UNARY_NEG,
	UNARY_NOT,
	UNARY_COUNT
} EyreUnaryOp;



static char* eyreUnaryOpSymbols[UNARY_COUNT] = {
	"+", "-", "~"
};



typedef enum EyreBinaryOp {
	BINARY_ADD,
	BINARY_SUB,
	BINARY_MUL,
	BINARY_DIV,
	BINARY_AND,
	BINARY_OR,
	BINARY_XOR,
	BINARY_SHL,
	BINARY_SHR,
	BINARY_COUNT
} EyreBinaryOp;



static char* eyreBinaryOpSymbols[BINARY_COUNT] = {
	"+", "-", "*", "/", "&", "|", "^", "<<", ">>"
};



typedef enum EyreTokenType {
	TOKEN_END,
	TOKEN_INT,
	TOKEN_LONG,
	TOKEN_CHAR,
	TOKEN_STRING,
	TOKEN_ID,

	TOKEN_PLUS,
	TOKEN_MINUS,
	TOKEN_EQUALS,
	TOKEN_EQUALITY,
	TOKEN_INEQUALITY,
	TOKEN_EXCLAMATION,
	TOKEN_LPAREN,
	TOKEN_RPAREN,
	TOKEN_LBRACKET,
	TOKEN_RBRACKET,
	TOKEN_LBRACE,
	TOKEN_RBRACE,
	TOKEN_SEMICOLON,
	TOKEN_COLON,
	TOKEN_LT,
	TOKEN_LTE,
	TOKEN_GT,
	TOKEN_GTE,
	TOKEN_DOT,
	TOKEN_SLASH,
	TOKEN_TILDE,
	TOKEN_PIPE,
	TOKEN_AMPERSAND,
	TOKEN_ASTERISK,
	TOKEN_REFERENCE,
	TOKEN_LOGICAL_AND,
	TOKEN_LOGICAL_OR,
	TOKEN_COMMA,
	TOKEN_LSHIFT,
	TOKEN_RSHIFT,
	TOKEN_CARET,

	TOKEN_COUNT
} EyreTokenType;



#define TOKEN_SYM_START TOKEN_PLUS



static char* eyreTokenNames[TOKEN_COUNT] = {
	"END","INT","LONG","CHAR","STRING","ID",
	"+","-","=","==",
	"!=","!","(",")",
	"[","]","{","}",
	";",":","<","<=",
	">",">=",".","/",
	"~","|","&","*",
	"::","&&","||",",",
	"<<", ">>", "^",
};



typedef enum RegisterType {
	REGISTER_BYTE,
	REGISTER_WORD,
	REGISTER_DWORD,
	REGISTER_QWORD,
	REGISTER_COUNT
} RegisterType;



static char* eyreByteRegNames[16] = {
	"al","cl","dl","bl","ah","ch","dh","bh",
	"r8b","r9b","r10b","r11b","r12b","r13b","r14b","r15b",
};

static char* eyreWordRegNames[16] = {
	"ax","cx","dx","bx","sp","bp","si","di",
	"r8w","r9w","r10w","r11w","r12w","r13w","r14w","r15w",
};

static char* eyreDWordRegNames[16] = {
	"eax","ecx","edx","ebx","esp","ebp","esi","edi",
	"r8d","r9d","r10d","r11d","r12d","r13d","r14d","r15d",
};

static char* eyreQWordRegNames[16] = {
	"rax","rcx","rdx","rbx","rsp","rbp","rsi","rdi",
	"r8","r9","r10","r11","r12","r13","r14","r15",
};



typedef enum {
	KEYWORD_CONST,
	KEYWORD_STRUCT,
	KEYWORD_NAMESPACE,
	KEYWORD_PROC,
	KEYWORD_COUNT,
} EyreKeyword;



static char* eyreKeywordNames[KEYWORD_COUNT] = {
	"const", "struct", "namespace", "proc"
};



typedef enum {
	WIDTH_BYTE,
	WIDTH_WORD,
	WIDTH_DWORD,
	WIDTH_QWORD,
	WIDTH_COUNT,
} EyreWidth;



static char* eyreWidthNames[WIDTH_COUNT] = {
	"byte", "word", "dword", "qword"
};



typedef enum EyreMnemonic {
	MNEMONIC_ADD8, MNEMONIC_OR8, MNEMONIC_ADC8, MNEMONIC_SBB8,
	MNEMONIC_AND8, MNEMONIC_SUB8, MNEMONIC_XOR8, MNEMONIC_CMP8,
	MNEMONIC_JMP8, MNEMONIC_ADD, MNEMONIC_OR, MNEMONIC_ADC,
	MNEMONIC_SBB, MNEMONIC_AND, MNEMONIC_SUB, MNEMONIC_XOR,
	MNEMONIC_CMP, MNEMONIC_PUSH, MNEMONIC_PUSH_FS, MNEMONIC_PUSH_GS,
	MNEMONIC_PUSHW_FS, MNEMONIC_PUSHW_GS, MNEMONIC_POP, MNEMONIC_POP_FS,
	MNEMONIC_POP_GS, MNEMONIC_POPW_FS, MNEMONIC_POPW_GS, MNEMONIC_MOVSXD,
	MNEMONIC_MOVSX, MNEMONIC_MOVZX, MNEMONIC_INSB, MNEMONIC_INSW,
	MNEMONIC_INSD, MNEMONIC_OUTSB, MNEMONIC_OUTSW, MNEMONIC_OUTSD,
	MNEMONIC_JA, MNEMONIC_JAE, MNEMONIC_JB, MNEMONIC_JBE,
	MNEMONIC_JC, MNEMONIC_JE, MNEMONIC_JG, MNEMONIC_JGE,
	MNEMONIC_JL, MNEMONIC_JLE, MNEMONIC_JNA, MNEMONIC_JNAE,
	MNEMONIC_JNB, MNEMONIC_JNBE, MNEMONIC_JNC, MNEMONIC_JNE,
	MNEMONIC_JNG, MNEMONIC_JNGE, MNEMONIC_JNL, MNEMONIC_JNLE,
	MNEMONIC_JNO, MNEMONIC_JNP, MNEMONIC_JNS, MNEMONIC_JNZ,
	MNEMONIC_JO, MNEMONIC_JP, MNEMONIC_JPE, MNEMONIC_JPO,
	MNEMONIC_JS, MNEMONIC_JZ, MNEMONIC_TEST, MNEMONIC_XCHG,
	MNEMONIC_MOV, MNEMONIC_LEA, MNEMONIC_NOP, MNEMONIC_CBW,
	MNEMONIC_CWDE, MNEMONIC_CDQE, MNEMONIC_CWD, MNEMONIC_CDQ,
	MNEMONIC_CQO, MNEMONIC_WAIT, MNEMONIC_FWAIT, MNEMONIC_PUSHF,
	MNEMONIC_PUSHFQ, MNEMONIC_LAHF, MNEMONIC_MOVSB, MNEMONIC_MOVSW,
	MNEMONIC_MOVSD, MNEMONIC_MOVSQ, MNEMONIC_CMPSB, MNEMONIC_CMPSW,
	MNEMONIC_CMPSD, MNEMONIC_CMPSQ, MNEMONIC_STOSB, MNEMONIC_STOSW,
	MNEMONIC_STOSD, MNEMONIC_STOSQ, MNEMONIC_SCASB, MNEMONIC_SCASW,
	MNEMONIC_SCASD, MNEMONIC_SCASQ, MNEMONIC_LODSB, MNEMONIC_LODSW,
	MNEMONIC_LODSD, MNEMONIC_LODSQ, MNEMONIC_ROL, MNEMONIC_ROR,
	MNEMONIC_RCL, MNEMONIC_RCR, MNEMONIC_SAL, MNEMONIC_SHL,
	MNEMONIC_SHR, MNEMONIC_SAR, MNEMONIC_RET, MNEMONIC_RETF,
	MNEMONIC_LEAVEW, MNEMONIC_LEAVE, MNEMONIC_INT3, MNEMONIC_INT,
	MNEMONIC_INT1, MNEMONIC_IRETW, MNEMONIC_IRETD, MNEMONIC_IRETQ,
	MNEMONIC_JCXZ, MNEMONIC_JRCXZ, MNEMONIC_IN, MNEMONIC_OUT,
	MNEMONIC_HLT, MNEMONIC_CMC, MNEMONIC_NOT, MNEMONIC_NEG,
	MNEMONIC_MUL, MNEMONIC_IMUL, MNEMONIC_DIV, MNEMONIC_IDIV,
	MNEMONIC_CLC, MNEMONIC_STC, MNEMONIC_CLI, MNEMONIC_STI,
	MNEMONIC_CLD, MNEMONIC_STD, MNEMONIC_INC, MNEMONIC_DEC,
	MNEMONIC_CALL, MNEMONIC_CALLF, MNEMONIC_JMP, MNEMONIC_JMPF,
	MNEMONIC_RDRAND, MNEMONIC_RDSEED,
	MNEMONIC_COUNT
} EyreMnemonic;

static char* eyreMnemonicNames[MNEMONIC_COUNT] = {
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



// Structs



typedef struct SrcFile {
	char* path;
	char* chars;
	int   size;
} SrcFile;



typedef struct {
	void* data;
	int size;
	int capacity;
} List;



typedef struct {
	int id;
	int hash;
	int length;
	int flags;
	char* string;
} Intern;



// General



void eyreCreateSrcFile(SrcFile* srcFile, char* path);

char* eyreGetLocalFile(char* fileName);



// Interning



static List eyreInternList;

int eyreAddIntern(char* string, int length, int copy);

Intern* eyreGetIntern(u32 id);

void eyreInitInterns();

extern int eyreRegisterInternStart;
extern int eyreRegisterInternCount;
extern int eyreRegisterInternEnd;

extern int eyreKeywordInternStart;
extern int eyreKeywordInternCount;
extern int eyreKeywordInternEnd;

extern int eyreWidthInternStart;
extern int eyreWidthInternCount;
extern int eyreWidthInternEnd;

extern int eyreMnemonicInternStart;
extern int eyreMnemonicInternCount;
extern int eyreMnemonicInternEnd;



// Lexing



void eyreLex(SrcFile* srcFile);

void eyrePrintTokens();



// Parsing



void eyreParse();

void eyrePrintNode(void* node);

void eyrePrintNodes();



// Allocation



void* eyreAlloc(int size);

void* eyreRealloc(void* pointer, int size);

void eyreFree(void* pointer);

void eyreCheckListCapacity(List* list, int elementSize);



#endif // EYRE_INCLUDE