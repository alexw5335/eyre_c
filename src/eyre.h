#ifndef EYRE_INCLUDE
#define EYRE_INCLUDE



#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include "defs.h"



// Temp



void eyreAssemble(SrcFile* srcFile);



// Util



void checkCapacity(void** pData, int size, int* pCapacity, int elementSize);

void printPointer(void* value);

void printInt(int value);

void printString(char* value);

void println(char* format, ...);

void printNewline();

void eyreRunCommand(char* command);

void eyreRunCommandArgs(int num, ...);

void eyreWriteFile(char* path, int dataSize, void* data);



// General



void eyreCreateSrcFile(SrcFile* srcFile, char* path);

char* eyreGetLocalFile(char* fileName);

EyreGroup* eyreGetEncodings(int mnemonic);



// Lexing



void eyreLex(SrcFile* srcFile);

void eyrePrintTokens();



// Allocation



void* eyreAlloc(int size);

void* eyreRealloc(void* pointer, int size);

void eyreFree(void* pointer);

void eyreCheckListCapacity(List* list, int elementSize);

void* eyreAllocPersistent(int size);

void eyreAllocPersistentContiguous(void** start, int size);



#endif // EYRE_INCLUDE