#include "eyre.h"
#include "parse.h"
#include "log.h"
#include "resolve.h"
#include "intern.h"



static int scopeStack[32];
static int scopeSize;



static SymBase* resolveSymbol(int name) {
	SymBase* globalSymbol = eyreResolveSymbol(0, name);
	if(globalSymbol != NULL)
		return globalSymbol;

	for(int i = scopeSize - 1; i >= 0; i--) {
		int scope = scopeStack[i];
		SymBase* symbol = eyreResolveSymbol(scope, name);
		if(symbol != NULL)
			return symbol;
	}

	eyreError("Could not resolve symbol: %s", eyreGetString(name)->data);
	return NULL;
}



static int getScope(void* symbol) {
	SymBase* base = symbol;

	if(base->type == SYM_NAMESPACE) {
		NamespaceSymbol* s = symbol;
		return s->thisScope;
	}

	eyreError("Invalid scoped symbol: %d", base->type);
	return 0;
}



static SymBase* resolveDot(DotNode* node) {
	int name = node->right->nameIntern;
	char leftType = *(char*) node->left;

	if(leftType == NODE_SYM) {
		SymNode* left = node->left;
		SymBase* scopedSymbol = resolveSymbol(left->nameIntern);
		int scope = getScope(scopedSymbol);
		SymBase* symbol = eyreResolveSymbol(scope, name);
		if(symbol == NULL)
			eyreError("Could not resolve symbol: %s", eyreGetString(name)->data);
		return symbol;
	}

	if(leftType == NODE_DOT) {
		return NULL;
	}

	eyreError("Invalid dot node: %d", leftType);
	return NULL;
}



static void resolveSymbols(void* n) {
	char type = *(char*) n;

	if(type == NODE_UNARY) {
		UnaryNode* node = n;
		resolveSymbols(node->value);
	} else if(type == NODE_BINARY) {
		BinaryNode* node = n;
		resolveSymbols(node->left);
		resolveSymbols(node->right);
	} else if(type == NODE_SYM) {
		SymNode* node = n;
		node->symbol = resolveSymbol(node->nameIntern);
	} else if(type == NODE_IMM) {
		ImmNode* node = n;
		resolveSymbols(node->value);
	} else if(type == NODE_DOT) {
		resolveDot(n);
	}
}



void eyreResolve(SrcFile* inputSrcFile) {
	scopeSize = 0;

	for(int i = 0; i < inputSrcFile->nodeCount; i++) {
		void* n = inputSrcFile->nodes[i];
		char type = *(char*) n;

		if(type == NODE_NAMESPACE) {
			NamespaceNode* node = n;
			scopeStack[scopeSize++] = node->symbol->thisScope;
		} else if(type == NODE_SCOPE_END) {
			scopeSize--;
		} else if(type == NODE_INS) {
			InsNode* node = n;
			if(node->op1 == NULL) continue;
			resolveSymbols(node->op1);
			if(node->op2 == NULL) continue;
			resolveSymbols(node->op2);
			if(node->op3 == NULL) continue;
			resolveSymbols(node->op3);
			if(node->op4 == NULL) continue;
			resolveSymbols(node->op4);
		}
	}
}
