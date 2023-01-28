#ifndef EYRE_INCLUDE
#define EYRE_INCLUDE



#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include "defs.h"



void checkCapacity(void** pData, int size, int* pCapacity, int elementSize);



// General



void eyreCreateSrcFile(SrcFile* srcFile, char* path);

char* eyreGetLocalFile(char* fileName);

EyreGroup eyreGetEncodings(int mnemonic);



// Lexing



void eyreLex(SrcFile* srcFile);

void eyrePrintTokens();



// Allocation



void* eyreAlloc(int size);

void* eyreRealloc(void* pointer, int size);

void eyreFree(void* pointer);

void eyreCheckListCapacity(List* list, int elementSize);

void* eyreAllocPersistent(int size);



#endif // EYRE_INCLUDE