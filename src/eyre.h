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



// Interning



static List eyreInternList;

int eyreAddIntern(char* string, int length, int copy);

Intern* eyreGetIntern(u32 id);

void eyreInitInterns();

extern int eyreRegisterInternStart;
extern int eyreRegisterInternCount;
extern int eyreRegisterInternEnd;

int eyreInternToRegister(int intern) {
	int value = intern - eyreRegisterInternStart;
	if(value < eyreRegisterInternCount) return value;
	return FALSE;
}

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



void eyreParse(SrcFile* srcFile);

void eyrePrintNode(void* node);

void eyrePrintNodes();



// Allocation



void* eyreAlloc(int size);

void* eyreRealloc(void* pointer, int size);

void eyreFree(void* pointer);

void eyreCheckListCapacity(List* list, int elementSize);



#endif // EYRE_INCLUDE