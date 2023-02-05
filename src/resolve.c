#include "eyre.h"
#include "parse.h"
#include "log.h"
#include "resolve.h"
#include "intern.h"



static int scopeStack[32];
static int scopeSize;
static SrcFile* srcFile;
static int pos = 0;



#define assemblerErrorOffset(format, offset, ...) resolveError_(format, offset, __FILE__, __LINE__, ##__VA_ARGS__)

#define assemblerError(format, ...) resolveError_(format, 1, __FILE__, __LINE__, ##__VA_ARGS__)

static void resolveError_(char* format, int offset, char* file, int line, ...) {
	if(pos - offset >= srcFile->nodeCount)
		eyreError("Invalid resolver error node index: %d", pos - offset);
	fprintf(stdout, "Resolver error at %s:%d: ", srcFile->path, srcFile->nodeLines[pos - offset]);
	va_list args;
	va_start(args, line);
	vfprintf(stdout, format, args);
	fprintf(stdout, "\n");
	eyreError_("Resolver error", file, line);
}



static SymBase* resolveSymbol(int name) {
	for(int i = scopeSize - 1; i >= 0; i--) {
		int scope = scopeStack[i];
		SymBase* symbol = eyreResolveSymbol(scope, name);
		if(symbol != NULL)
			return symbol;
	}

	assemblerError("Could not resolve symbol: %s", eyreGetString(name)->data);
	return NULL;
}



static int getScope(void* symbol) {
	SymBase* base = symbol;

	if(base->type == SYM_NAMESPACE) {
		NamespaceSymbol* s = symbol;
		return s->thisScope;
	}

	assemblerError("Invalid scoped symbol: %d", base->type);
	return 0;
}



static SymBase* resolveDot(DotNode* node) {
	int name = node->right->nameIntern;
	char leftType = *(char*) node->left;
	SymBase* symbol;

	if(leftType == NODE_SYM) {
		SymNode* left = node->left;
		SymBase* scopedSymbol = resolveSymbol(left->nameIntern);
		int scope = getScope(scopedSymbol);
		symbol = eyreResolveSymbol(scope, name);
	} else if(leftType == NODE_DOT) {
		SymBase* left = resolveDot(node->left);
		int scope = getScope(left);
		symbol = eyreResolveSymbol(scope, name);
	} else {
		assemblerError("Invalid dot node");
		return NULL;
	}

	if(symbol == NULL)
		assemblerError("Could not resolve symbol: %s", eyreGetString(name)->data);
	return symbol;
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
		DotNode* node = n;
		resolveDot(node);
		*(&n) = node->right;
	}
}



void eyreResolve(SrcFile* inputSrcFile) {
	int prevPos = pos;
	SrcFile* prevSrcFile = srcFile;
	pos = 0;
	srcFile = inputSrcFile;

	scopeStack[0] = 0;
	scopeSize = 1;

	while(pos < srcFile->nodeCount) {
		void* n = srcFile->nodes[pos++];
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

	pos = prevPos;
	srcFile = prevSrcFile;
}
