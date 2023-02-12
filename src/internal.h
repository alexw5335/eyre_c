#ifndef INCLUDE_INTERNAL
#define INCLUDE_INTERNAL


#include <stdarg.h>
#include <stdio.h>
#include <mem.h>
#include <stdlib.h>
#include "symbols.h"



// Uncategorised




// Primitives



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



#define I8_MIN (-128)
#define I16_MIN (-32768)
#define I32_MIN (-2147483647 - 1)
#define I64_MIN  (-9223372036854775807LL - 1)

#define I8_MAX 127
#define I16_MAX 32767
#define I32_MAX 2147483647
#define I64_MAX 9223372036854775807LL

#define U8_MAX 255
#define U16_MAX 65535
#define U32_MAX 0xffffffffU
#define U64_MAX 0xffffffffffffffffULL


int isImm8(s64 input);

int isImm16(s64 input);

int isImm32(s64 input);


// Structs



typedef struct {
	char type;
	char value;
} Register;



typedef struct SrcFile {
	char*  path;
	int    tokenCount;
	char*  tokenTypes;
	int*   tokenValues;
	short* tokenLines;
	u8*    newlines;
	u8*    terminators;
	void** nodes;
	short* nodeLines;
	int    nodeCount;
} SrcFile;



typedef struct {
	int hash;
	int length;
	char* data;
} StringIntern;



typedef struct {
	int hash;
	int length;
	int* data;
} ScopeIntern;



typedef struct {
	int dllName;
	int importCount;
	int importCapacity;
	DllImportSymbol** imports;
} DllImport;



// Types of relocation:
// Rip-relative (uses pos, width, node, and offset, first positive reference used as absolute)
// Absolute (uses pos, width, and node, first positive reference used as absolute)
// Other (uses pos, width, and node, no special calculation needed)
typedef struct {
	int   pos;
	char  width;
	void* node;
	int   offset; // >= 0: RIP-relative. -1: Absolute. -2: Other
} Relocation;



// Variables



#define dllImportCapacity 64

extern DllImport dllImports[dllImportCapacity];

extern int dllImportCount;

#define relocationCapacity 512

extern Relocation relocations[relocationCapacity];

extern int relocationCount;

void* getAssemblerBuffer();

int getAssemblerBufferLength();

void* getLinkerBuffer();

int getLinkerBufferLength();



// Memory



void* palloc(int size); // persistent allocation

void checkCapacity(void** pData, int size, int* pCapacity, int elementSize);



// Logging



void printPointer(void* value);

void printInt(int value);

void printString(char* value);

void println(char* format, ...);

void printNewline();

void errorAt(char* format, const char* file, int line, ...);

#define error(format, ...) errorAt(format, __FILE__, __LINE__, ##__VA_ARGS__)



// Commands



void runCommand(char* command);

void runCommandArgs(int num, ...);



// Files



void readFile(char* path);

int getReadFileLength();

char* getReadFileData();

char* getLocalFile(char* fileName);

void writeFile(char* path, int dataSize, void* data);



// Compilation



void eyreLex(SrcFile* srcFile);

void eyrePrintTokens(SrcFile* srcFile);

void eyreParse(SrcFile* srcFile);

void eyrePrintNode(void* node);

void eyrePrintNodes(SrcFile* srcFile);

void eyreResolve(SrcFile* srcFile);

void eyreAssemble(SrcFile* srcFile);

void eyreLink();



#endif
