#ifndef EYRE_INCLUDE
#define EYRE_INCLUDE



#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include "defs.h"



// General



void eyreCreateSrcFile(SrcFile* srcFile, char* path);

char* eyreGetLocalFile(char* fileName);

extern EyreGenGroup eyreEncodings[];



// Lexing



void eyreLex(SrcFile* srcFile);

void eyrePrintTokens();



// Parsing



void eyreParse(SrcFile* srcFile);

void eyrePrintNode(void* node);

void eyrePrintNodes();



// Allocation



void* eyreAlloc(int size);

void* eyreRealloc(void* pointer, int size);

void eyreFree(void* pointer);

void eyreCheckListCapacity(List* list, int elementSize);

void* eyreAllocPersistent(int size);



#endif // EYRE_INCLUDE