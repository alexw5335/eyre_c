#ifndef EYRE_INTERN_INCLUDE
#define EYRE_INTERN_INCLUDE

#include "defs.h"
#include "symbol.h"



int eyreInternString(char* data, int length);

int eyreInternScope(int* data, int length, int hash);

int eyreInternSymbol(SymBase* symbol);

void* eyreResolveSymbol(int scope, int name);

StringIntern* eyreGetString(int id);

ScopeIntern* eyreGetScope(int id);

SymBase* eyreGetSymbol(int id);

void eyrePrintSymbols();

void eyreInitInterns();

int eyreInternToRegister(int intern);

int eyreInternToKeyword(int intern);

int eyreInternToWidth(int intern);

int eyreInternToMnemonic(int intern);



#endif
