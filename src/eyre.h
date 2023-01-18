#ifndef EYRE_INCLUDE
#define EYRE_INCLUDE



#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>



// Types



typedef unsigned char      u8;
typedef signed char        s8;
typedef unsigned short     u16;
typedef signed short       s16;
typedef unsigned int       u32;
typedef signed int         s32;
typedef unsigned long long u64;
typedef signed long long   s64;



// Enums



typedef enum EyreTokenType {
	T_END,
	T_INT,
	T_LONG,
	T_CHAR,
	T_STRING,
	T_ID,
	T_SYM,
	T_COUNT
} EyreTokenType;



typedef enum EyreTokenSymbol {
	S_PLUS,
	S_MINUS,
	S_EQUALS,
	S_EQUALITY,
	S_INEQUALITY,
	S_EXCLAMATION,
	S_LPAREN,
	S_RPAREN,
	S_LBRACKET,
	S_RBRACKET,
	S_LBRACE,
	S_RBRACE,
	S_SEMICOLON,
	S_COLON,
	S_LT,
	S_LTE,
	S_GT,
	S_GTE,
	S_DOT,
	S_SLASH,
	S_TILDE,
	S_PIPE,
	S_AMPERSAND,
	S_ASTERISK,
	S_REFERENCE,
	S_LOGICAL_AND,
	S_LOGICAL_OR,
	S_COMMA,
	S_COUNT
} EyreTokenSymbol;



static char* eyreTokenSymbolNames[S_COUNT] = {
	"+","-","=","==",
	"!=","!","(",")",
	"[","]","{","}",
	";",":","<","<=",
	">",">=",".","/",
	"~","|","&","*",
	"::","&&","||",","
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


// TODO: Auto-generate a list of valid mnemonics from an instruction table
typedef enum {
	ADD,
	SUB,
} EyreMnemonic;




// Structs



typedef struct SrcFile {
	char* path;
	char* chars;
	int   size;
	u8*   tokenTypes;
	u32*  tokens;
	int   tokenCount;
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



void eyreCreateSrcFileFromFile(SrcFile* srcFile, char* path);

char* eyreGetFileInCurrentDirectory(char* fileName);



// Interning



static List eyreInternList;

int eyreAddIntern(char* string, int length);

Intern* eyreGetIntern(u32 id);

void eyreInitInterns();

static int eyreRegisterInternStart;
static int eyreRegisterInternEnd;

static int eyreKeywordInternStart;
static int eyreKeywordInternEnd;

static int eyreWidthInternStart;
static int eyreWidthInternEnd;



// Lexing



void eyreLex(SrcFile* srcFile);

void eyrePrintTokens();



// Allocation



void* eyreAlloc(int size);

void* eyreRealloc(void* pointer, int size);

void eyreFree(void* pointer);

void eyreCheckListCapacity(List* list, int elementSize);



#endif // EYRE_INCLUDE