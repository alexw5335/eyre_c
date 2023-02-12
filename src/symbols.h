#ifndef INCLUDE_SYMBOLS
#define INCLUDE_SYMBOLS



typedef struct {
	char  type;
	char  flags;
	int   scope;
	int   name;
} SymBase;



typedef struct {
	SymBase base;
	int pos;
} PosSymbol;



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



typedef struct {
	SymBase  base;
	int      pos;
	int      importName;
} DllImportSymbol;



#endif