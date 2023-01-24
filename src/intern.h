#ifndef EYRE_INTERN_INCLUDE
#define EYRE_INTERN_INCLUDE

#include "defs.h"

int eyreAddIntern(char* string, int length, int copy);

Intern* eyreGetIntern(u32 id);

void eyreInitInterns();

int eyreInternToRegister(int intern);

int eyreInternToKeyword(int intern);

int eyreInternToWidth(int intern);

int eyreInternToMnemonic(int intern);

#endif
