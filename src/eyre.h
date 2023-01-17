#pragma once

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
	char* data;
	int size;
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



typedef struct {
	int intern;
	int next;
} Node;




// Functions



void createSrcFile(SrcFile* srcFile, char* path);

void eyreLex(SrcFile* srcFile);

int eyreIntern(char* string, int length);

void printTokens();

void listEnsureCapacity(List* list, int elementSize);

Intern* getIntern(u32 id);

char* getFileInCurrentDirectory(char* fileName);



// Allocation



void* eyreAlloc(int size);

void* eyreRealloc(void* pointer, int size);

void eyreFree(void* pointer);

void* eyreAllocPersistent(int size);

void checkListCapacity(List* list, int elementSize);



// PRINTING



void printPointer(void* value);

void printInt(int value);

void printString(char* value);

void println(char* format, ...);



// LOGGING



// Always results in termination, logged to stderr
void eyreLogError_(char* format, const char* file, int line, ...);

// Always logged
void eyreLogWarning_(char* format, const char* file, int line, ...);

// Important debug messages
void eyreLogDebug_(char* format, const char* file, int line, ...);

// Unimportant debug messages
void eyreLogInfo_(char* format, const char* file, int line, ...);

// Only for targeted debugging
void eyreLogTrace_(char* format, const char* file, int line, ...);

#define eyreLogError(format, ...) eyreLogError_(format, __FILE__, __LINE__, ##__VA_ARGS__)

#define eyreLogWarning(format, ...) eyreLogWarning_(format, __FILE__, __LINE__, ##__VA_ARGS__)

#if defined(EYRE_LOG_DEBUG) | defined(EYRE_LOG_ALL)
#define eyreLogDebug(format, ...) eyreLogDebug_(format, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define eyreLogDebug(format, ...)
#endif

#if defined(EYRE_LOG_INFO) | defined(EYRE_LOG_ALL)
#define eyreLogInfo(format, ...) eyreLogInfo_(format, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define eyreLogInfo(format, ...)
#endif

#if defined(EYRE_LOG_TRACE) | defined(EYRE_LOG_ALL)
#define eyreLogTrace(format, ...) eyreLogTrace_(format, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define eyreLogTrace(format, ...)
#endif