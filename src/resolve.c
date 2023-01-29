#include "eyre.h"
#include "parse.h"
#include "log.h"
#include "resolve.h"
#include "intern.h"



static int scopeStack[32];
static int scopeSize;
static int currentScope;



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
		int name = node->nameIntern;
		node->symbol = eyreResolveSymbol(currentScope, name);
		if(node->symbol == NULL)
			eyreError("Could not resolve symbol");
	} else if(type == NODE_IMM) {
		ImmNode* node = n;
		resolveSymbols(node->value);
	}
}



void eyreResolve(SrcFile* inputSrcFile) {
	currentScope = 1;
	scopeSize = 1;

	for(int i = 0; i < inputSrcFile->nodeCount; i++) {
		void* n = inputSrcFile->nodes[i];
		char type = *(char*) n;

		if(type == NODE_NAMESPACE) {
			NamespaceNode* node = n;
			currentScope = node->symbol->base.scope;
			scopeStack[scopeSize++] = currentScope;
		} else if(type == NODE_SCOPE_END) {
			currentScope = scopeStack[--scopeSize];
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
