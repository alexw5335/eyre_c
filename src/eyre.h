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
	"+",
	"-",
	"=",
	"==",
	"!=",
	"!",
	"(",
	")",
	"[",
	"]",
	"{",
	"}",
	";",
	":",
	"<",
	"<=",
	">",
	">=",
	".",
	"/",
	"~",
	"|",
	"&",
	"*",
	"::",
	"&&",
	"||",
	","
};


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



int eyreAddIntern(char* string, int length);

Intern* eyreGetIntern(u32 id);



// Lexing



void eyreLex(SrcFile* srcFile);

void eyrePrintTokens();



// Allocation



void* eyreAlloc(int size);

void* eyreRealloc(void* pointer, int size);

void eyreFree(void* pointer);

void eyreCheckListCapacity(List* list, int elementSize);



#endif // EYRE_INCLUDE