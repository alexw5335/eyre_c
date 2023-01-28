#include "eyre.h"
#include "symbol.h"
#include "intern.h"

void* eyreAddSymbol(char type, int scope, int name, int size) {
	SymBase* symbol = eyreAllocPersistent(size);
	symbol->type = type;
	symbol->scope = scope;
	symbol->name = name;
	eyreInternSymbol(symbol);
	return symbol;
}
