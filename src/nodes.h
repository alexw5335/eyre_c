#ifndef INCLUDE_NODES
#define INCLUDE_NODES



#include "symbols.h"



#define nodeType(pointer) *(char*)pointer



typedef struct {
	char type;
	int value;
} IntNode;



typedef struct {
	char type;
	char width;
	char value;
} RegNode;



typedef struct {
	char  type;
	int   nameIntern;
	void* symbol;
} SymNode;



typedef struct {
	char type;
	char op;
	void* value;
} UnaryNode;



typedef struct {
	char  type;
	char  op;
	void* left;
	void* right;
} BinaryNode;



typedef struct {
	char  type;
	char  mnemonic;
	char  size;
	void* op1;
	void* op2;
	void* op3;
	void* op4;
} InsNode;



typedef struct {
	char type;
	char width;
	void* value;
} MemNode;



typedef struct {
	char type;
	void* value;
} ImmNode;



typedef struct {
	int name;
	int typeName;
} StructMemberNode;



typedef struct {
	char type;
	int name;
	int symbol;
	StructMemberNode* members;
} StructNode;



typedef struct {
	int name;
	void* expression;
} EnumEntryNode;



typedef struct {
	char type;
	int name;
	int symbol;
	EnumEntryNode* nodes;
} EnumNode;



typedef struct {
	char type;
	int name;
	NamespaceSymbol* symbol;
} NamespaceNode;



typedef struct {
	char type;
} ScopeEndNode;



typedef struct {
	char type;
	LabelSymbol* symbol;
} LabelNode;



typedef struct {
	char     type;
	void*    left;
	SymNode* right;
} DotNode;



typedef struct {
	char   type;
	int    argCount;
	void** args;
	void*  invoker;
} InvokeNode;



typedef struct {
	char   width;
	int    nodeCount;
	void** nodes;
} VarPart;



typedef struct {
	char      type;
	int       partCount;
	VarPart** parts;
} VarNode;



#endif