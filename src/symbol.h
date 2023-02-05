#ifndef EYRE_SYMBOL_INCLUDE
#define EYRE_SYMBOL_INCLUDE



typedef enum {
	SYM_STRUCT,
	SYM_ENUM,
	SYM_NAMESPACE,
	SYM_CONST,
	SYM_DLL_IMPORT,
	SYM_LABEL,
} EyreSymbolType;


typedef enum {
	SYM_FLAGS_POS = 1
} EyreSymbolFlagBits;



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



typedef struct {
	int dllName;
	int importCount;
	int importCapacity;
	DllImportSymbol** imports;
} DllImport;



void* eyreAddSymbol(char type, int scope, int name, int size);



#endif