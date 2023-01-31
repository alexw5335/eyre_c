#ifndef EYRE_SYMBOL_INCLUDE
#define EYRE_SYMBOL_INCLUDE



typedef enum {
	SYM_STRUCT,
	SYM_ENUM,
	SYM_NAMESPACE,
	SYM_CONST,
	SYM_LABEL,
} EyreSymbolType;



typedef struct {
	char  type;
	int   scope;
	int   name;
} SymBase;



typedef struct {
	SymBase base;
	int     file;
	int     value;
} ConstSymbol;



typedef struct {
	SymBase base;
	int pos;
} LabelSymbol;



typedef struct {
	SymBase base;
	int thisScope;
} NamespaceSymbol;



void* eyreAddSymbol(char type, int scope, int name, int size);



#endif