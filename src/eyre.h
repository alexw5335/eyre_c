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



typedef enum {
	MNEMONIC_ADD,
	MNEMONIC_SUB,
	MNEMONIC_COUNT,
} EyreMnemonic;

static char* eyreMnemonicNames[MNEMONIC_COUNT] = {
	"add", "sub"
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