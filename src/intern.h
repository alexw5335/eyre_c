#ifndef INCLUDE_INTERN
#define INCLUDE_INTERN

#include "symbols.h"
#include "internal.h"



int eyreInternString(char* data, int length);

int eyreInternScope(int* data, int length, int hash);

int eyreInternSymbol(SymBase* symbol);

void* eyreAddSymbol(char type, int scope, int name, int size);

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

int eyreInternToVarWidth(int intern);



extern int EYRE_INTERN_MAIN;



#endif
