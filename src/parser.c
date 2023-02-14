#include "internal.h"
#include "enums.h"
#include "nodes.h"
#include "intern.h"
#include "mnemonics.h"



// Variables



static SrcFile* srcFile;
static int pos;
static char* tokenTypes;
static int* tokenValues;

#define nodeCapacity 8192
static void* nodes[nodeCapacity];
static int nodeCount;
static short nodeLines[nodeCapacity];

#define scopesCapacity 64
static int scopeInterns[scopesCapacity];
static int scopesSize = 0;
static int currentScope = 0;
static int currentNamespace = 0; // Only for single-line namespaces



// Forward declarations



static void parseScope(int scope);

static void* parseExpression(int precedence);



// Errors



static void parserErrorAt(char* format, int offset, char* file, int line, ...) {
	if(pos - offset >= srcFile->tokenCount)
		error("Invalid parser error offset: %d", offset);
	fprintf(stdout, "Parser error at %s:%d: ", srcFile->path, srcFile->tokenLines[pos - offset]);
	va_list args;
	va_start(args, line);
	vfprintf(stdout, format, args);
	fprintf(stdout, "\n");
	errorAt("Parser error", file, line);
}

#define parserErrorOffset(format, offset, ...) parserErrorAt(format, offset, __FILE__, __LINE__, ##__VA_ARGS__)

#define parserError(format, ...) parserErrorAt(format, 1, __FILE__, __LINE__, ##__VA_ARGS__)



// Buffer



#define listBuilderCapacity 1024
static void* listBuilderData[listBuilderCapacity];
static int listBuilderSize = 0;



static void* listBuilderBuild() {
	void* data = palloc(listBuilderSize * sizeof(void*));
	memcpy(data, listBuilderData, listBuilderSize);
	return data;
}



static void listBuilderAdd(void* pointer) {
	if(listBuilderSize >= listBuilderCapacity)
		parserError("Too many list elements: %d", listBuilderSize);
	listBuilderData[listBuilderSize++] = pointer;
}



static void listBuilderBegin() {
	listBuilderSize = 0;
}



// Nodes



static inline void addNode(void* node) {
	if(nodeCount >= nodeCapacity)
		parserError("Too many nodes");

	if(pos >= srcFile->tokenCount)
		nodeLines[nodeCount] = srcFile->tokenLines[srcFile->tokenCount - 1];
	else
		nodeLines[nodeCount] = srcFile->tokenLines[pos - 1];

	nodes[nodeCount++] = node;
}



static inline void* createNode(int size, EyreNodeType type) {
	void* node = palloc(size);
	*(char*) node = type;
	return node;
}



static inline void addScopeEndNode() {
	addNode(createNode(sizeof(ScopeEndNode), NODE_SCOPE_END));
}



// Parsing utils



static inline int atTerminator() {
	return srcFile->terminators[pos >> 3] & (1 << (pos & 7));
}

static inline int atNewline() {
	return srcFile->newlines[pos >> 3] & (1 << (pos & 7));
}

static inline int parseId() {
	if(tokenTypes[pos] != TOKEN_ID)
		parserError("Expecting: TOKEN_ID, found: %s", eyreTokenNames[tokenTypes[pos]]);
	return tokenValues[pos++];
}

static void expectToken(EyreTokenType type) {
	if(tokenTypes[pos++] != type)
		parserError("Expecting: %s, found: %s", eyreTokenNames[type], eyreTokenNames[tokenTypes[pos - 1]]);
}

static void expectTerminator() {
	if(!atTerminator())
		parserError("Expecting statement end");
}



// Expression parsing



static int opPrecedence(char op) {
	switch(op) {
		case BINARY_DOT: return 6;
		case BINARY_INV: return 5;
		case BINARY_MUL:
		case BINARY_DIV: return 4;
		case BINARY_ADD:
		case BINARY_SUB: return 3;
		case BINARY_SHL:
		case BINARY_SHR: return 2;
		case BINARY_AND:
		case BINARY_OR:
		case BINARY_XOR: return 1;
		default: return 0;
	}
}



static char getBinaryOp(char tokenType) {
	switch(tokenType) {
		case TOKEN_ASTERISK:  return BINARY_MUL;
		case TOKEN_SLASH:     return BINARY_DIV;
		case TOKEN_PLUS:      return BINARY_ADD;
		case TOKEN_MINUS:     return BINARY_SUB;
		case TOKEN_LSHIFT:    return BINARY_SHL;
		case TOKEN_RSHIFT:    return BINARY_SHR;
		case TOKEN_AMPERSAND: return BINARY_AND;
		case TOKEN_PIPE:      return BINARY_OR;
		case TOKEN_CARET:     return BINARY_XOR;
		case TOKEN_DOT:       return BINARY_DOT;
		case TOKEN_LPAREN:    return BINARY_INV;
		default:              return -1;
	}
}



static void* parseAtom() {
	char type = tokenTypes[pos];
	int token = (int) tokenValues[pos++];

	if(type == TOKEN_ID) {
		int reg = eyreInternToRegister(token);
		if(reg >= 0) {
			RegNode* node = createNode(sizeof(RegNode), NODE_REG);
			node->width = reg >> 4;
			node->value = reg & 15;
			return node;
		}

		SymNode* node = createNode(sizeof(SymNode), NODE_SYM);
		node->nameIntern = token;
		return node;
	}

	if(type == TOKEN_INT) {
		IntNode* node = createNode(sizeof(IntNode), NODE_INT);
		node->value = token;
		return node;
	}

	if(type >= TOKEN_SYM_START) {
		if(type == TOKEN_LPAREN) {
			void* node = parseExpression(0);
			if(tokenTypes[pos++] != TOKEN_RPAREN)
				parserError("Expecting ')'");
			return node;
		}

		if(type == TOKEN_MINUS) {
			UnaryNode* node = createNode(sizeof(UnaryNode), NODE_UNARY);
			node->op = UNARY_NEG;
			node->value = parseAtom();
			return node;
		}

		if(type == TOKEN_PLUS) {
			UnaryNode* node = createNode(sizeof(UnaryNode), NODE_UNARY);
			node->op = UNARY_POS;
			node->value = parseAtom();
			return node;
		}

		if(type == TOKEN_TILDE) {
			UnaryNode* node = createNode(sizeof(UnaryNode), NODE_UNARY);
			node->op = UNARY_NOT;
			node->value = parseAtom();
			return node;
		}

		parserError("Invalid atom token (type = %s, token = %d)", eyreTokenNames[type], token);
	}

	parserError("Invalid atom token (type = %s, token = %d)", eyreTokenNames[type], token);
	return NULL;
}



static void* parseExpression(int precedence) {
	void* atom = parseAtom();

	while(TRUE) {
		char tokenType = tokenTypes[pos];

		if(tokenType < TOKEN_SYM_START) {
			if(!atTerminator()) {
				parserError("Use a semicolon to separate expressions that are on the same line");
			} else {
				break;
			}
		}

		if(tokenType == TOKEN_SEMICOLON) break;

		char op = getBinaryOp(tokenType);
		if(op == -1) break;
		int precedence2 = opPrecedence(op);
		if(precedence2 < precedence) break;
		pos++;

		if(op == BINARY_DOT) {
			DotNode* node = createNode(sizeof(DotNode), NODE_DOT);
			node->left = atom;
			node->right = parseExpression(precedence2 + 1);
			if(node->right->type != NODE_SYM)
				parserError("Expected id node, found: %s", eyreNodeNames[node->right->type]);
			atom = node;
		} else if(op == BINARY_INV) {
			InvokeNode* node = createNode(sizeof(InvokeNode), NODE_INVOKE);
			node->invoker = atom;
			atom = node;

			listBuilderBegin();

			while(1) {
				if(tokenTypes[pos] == TOKEN_RPAREN) break;
				listBuilderAdd(parseExpression(0));
				if(tokenTypes[pos] != TOKEN_COMMA) break;
				pos++;
			}

			expectToken(TOKEN_RPAREN);

			node->args = listBuilderBuild();
			node->argCount = listBuilderSize;
		} else {
			BinaryNode* node = createNode(sizeof(BinaryNode), NODE_BINARY);
			node->op = op;
			node->left = atom;
			node->right = parseExpression(precedence2 + 1);
			atom = node;
		}
	}

	return atom;
}



// Keyword parsing



static int addScope(int name) {
	if(scopesSize >= scopesCapacity)
		parserError("Too many scopes");

	int hash = 0;
	scopeInterns[scopesSize++] = name;

	for(int i = 0; i < scopesSize; i++)
		hash = hash * 31 + scopeInterns[i];

	return eyreInternScope(scopeInterns, scopesSize, hash);
}



static void parseDllImport() {
	int dllName = parseId();
	expectToken(TOKEN_LBRACE);

	DllImport* import = NULL;

	for(int i = 0; i < dllImportCount; i++) {
		if(dllImports[i].dllName == dllName) {
			import = &dllImports[i];
			break;
		}
	}

	if(import == NULL) {
		if(dllImportCount >= dllImportCapacity)
			parserError("Too many DLL imports: %d", dllImportCount);
		import = &dllImports[dllImportCount++];
	}

	while(tokenTypes[pos] != TOKEN_RBRACE) {
		int importName = parseId();
		expectTerminator();
		if(tokenTypes[pos] == TOKEN_COMMA) pos++;
		DllImportSymbol* symbol = eyreAddSymbol(SYM_DLL_IMPORT, currentScope, importName, sizeof(DllImportSymbol));
		symbol->base.flags = SYM_FLAGS_POS;
		symbol->importName = importName;
		checkCapacity((void**) &import->imports, import->importCount, &import->importCapacity, sizeof(void*));
		import->imports[import->importCount++] = symbol;
	}

	pos++;
}



static void parseNamespace() {
	int name = parseId();
	int thisScope = addScope(name);

	NamespaceSymbol* symbol = (NamespaceSymbol*) eyreAddSymbol(SYM_NAMESPACE, currentScope, name, sizeof(NamespaceSymbol));
	NamespaceNode* node = createNode(sizeof(NamespaceNode), NODE_NAMESPACE);
	node->name = name;
	node->symbol = symbol;
	node->symbol->thisScope = thisScope;

	if(tokenTypes[pos] == TOKEN_LBRACE) {
		pos++;
		addNode(node);
		parseScope(thisScope);
		expectToken(TOKEN_RBRACE);
		addScopeEndNode();
	} else {
		expectTerminator();
		if(currentNamespace != 0)
			addScopeEndNode();
		addNode(node);
		parseScope(thisScope);
		currentNamespace = node->symbol->thisScope;
	}
}



static void parseLabel(int name) {
	LabelSymbol* symbol = eyreAddSymbol(SYM_LABEL, currentScope, name, sizeof(LabelSymbol));
	symbol->base.flags = SYM_FLAGS_POS;
	LabelNode* node = createNode(sizeof(LabelNode), NODE_LABEL);
	node->symbol = symbol;
	addNode(node);
}



static void parseKeyword(EyreKeyword keyword) {
	switch(keyword) {
		case KEYWORD_NAMESPACE: parseNamespace(); break;
		case KEYWORD_DLLIMPORT: parseDllImport(); break;
		default: parserError("Invalid keyword: %s", eyreKeywordNames[keyword]);
	}
}



// Instruction parsing



static void* parseOperand() {
	char tokenType = tokenTypes[pos];
	char token = tokenValues[pos];
	int width = -1;

	if(tokenType == TOKEN_ID) {
		width = eyreInternToWidth(token);
		if(width < 0)
			width = -1;
		if(tokenTypes[pos + 1] == TOKEN_LBRACKET)
			tokenType = tokenTypes[++pos];
	}

	if(tokenType == TOKEN_LBRACKET) {
		pos++;
		void* value = parseExpression(0);
		if(tokenTypes[pos++] != TOKEN_RBRACKET)
			parserError("Expecting ']'");
		MemNode* node = createNode(sizeof(MemNode), NODE_MEM);
		node->width = width;
		node->value = value;
		return node;
	}

	void* node = parseExpression(0);
	char nodeType = *(char*) node;

	if(nodeType == NODE_REG)
		return node;

	ImmNode* immNode = createNode(sizeof(ImmNode), NODE_IMM);
	immNode->value = node;
	return immNode;
}



static InsNode* parseInstruction(EyreMnemonic mnemonic) {
	InsNode* node = createNode(sizeof(InsNode), NODE_INS);
	node->mnemonic = mnemonic;
	node->size = 0;
	node->op1 = NULL;
	node->op2 = NULL;
	node->op3 = NULL;
	node->op4 = NULL;

	if(atNewline() || tokenTypes[pos] == TOKEN_END) return node;

	node->op1 = parseOperand();
	node->size = 1;
	if(tokenTypes[pos] != TOKEN_COMMA) return node;
	pos++;

	node->op2 = parseOperand();
	node->size = 2;
	if(tokenTypes[pos] != TOKEN_COMMA) return node;
	pos++;

	node->op3 = parseOperand();
	node->size = 3;
	if(tokenTypes[pos] != TOKEN_COMMA) return node;
	pos++;

	node->op4 = parseOperand();
	node->size = 4;
	if(!atTerminator()) parserError("Expecting statement bufferEnd");
	return node;
}



// Scope parsing



/**
 * Must be paired with addScope
 */
static void parseScope(int scope) {
	int prevScope = currentScope;
	currentScope = scope;

	while(1) {
		u8 type = tokenTypes[pos];
		u32 token = tokenValues[pos];

		if(type == TOKEN_SEMICOLON) {
			pos++;
			continue;
		}

		if(type == TOKEN_RBRACE)
			break;

		if(type == TOKEN_END) {
			pos++;
			break;
		}

		if(type == TOKEN_ID) {
			pos++;

			if(tokenTypes[pos] == TOKEN_COLON) {
				pos++;
				parseLabel(token);
				continue;
			}

			int keyword = eyreInternToKeyword(token);

			if(keyword >= 0) {
				parseKeyword(keyword);
				continue;
			}

			int mnemonic = eyreInternToMnemonic(token);

			if(mnemonic >= 0) {
				addNode(parseInstruction(mnemonic));
				continue;
			}

			parserError("Invalid token (type = %s, token = %d)", eyreTokenNames[type], token);
		}

		parserError("Invalid token (type = %s, token = %d)", eyreTokenNames[type], token);

	}

	currentScope = prevScope;
	scopesSize--;
}




// Public functions



void eyreParse(SrcFile* inputSrcFile) {
	pos = 0;
	nodeCount = 0;
	srcFile = inputSrcFile;
	tokenTypes = srcFile->tokenTypes;
	tokenValues = srcFile->tokenValues;

	parseScope(0);

	if(currentNamespace != 0)
		addScopeEndNode();

	srcFile->nodes = malloc(nodeCount * 8);
	memcpy(srcFile->nodes, nodes, nodeCount * 8);

	srcFile->nodeLines = malloc(nodeCount * 2);
	memcpy(srcFile->nodeLines, nodeLines, nodeCount * 2);

	srcFile->nodeCount = nodeCount;
}



void eyrePrintNode(void* node) {
	char type = *(char*)node;

	if(type == NODE_INT) {
		IntNode* n = node;
		printf("%d", n->value);
	} else if(type == NODE_UNARY) {
		UnaryNode* n = node;
		printf("(%s", eyreUnaryOpSymbols[n->op]);
		eyrePrintNode(n->value);
		printf(")");
	} else if(type == NODE_BINARY) {
		BinaryNode* n = node;
		printf("(");
		eyrePrintNode(n->left);
		printf(" %s ", eyreBinaryOpSymbols[n->op]);
		eyrePrintNode(n->right);
		printf(")");
	} else if(type == NODE_REG) {
		RegNode* n = node;

		if(n->width == WIDTH_BYTE) {
			printf("%s", eyreByteRegNames[n->value]);
		} else if(n->width == WIDTH_WORD) {
			printf("%s", eyreWordRegNames[n->value]);
		} else if(n->width == WIDTH_DWORD) {
			printf("%s", eyreDWordRegNames[n->value]);
		} else if(n->width == WIDTH_QWORD) {
			printf("%s", eyreQWordRegNames[n->value]);
		} else {
			parserError("Invalid register");
		}
	} else if(type == NODE_MEM) {
		MemNode* n = node;
		if(n->width > 0)
			printf("%s [", eyreWidthNames[n->width]);
		else
			printf("[");
		eyrePrintNode(n->value);
		printf("]");
	} else if(type == NODE_IMM) {
		ImmNode* n = node;
		eyrePrintNode(n->value);
	} else if(type == NODE_INS) {
		InsNode* n = node;
		printf("%s", eyreMnemonicNames[n->mnemonic]);
		if(n->op1 == NULL) { printf("\n"); return; }
		printf(" ");
		eyrePrintNode(n->op1);
		if(n->op2 == NULL) { printf("\n"); return; }
		printf(", ");
		eyrePrintNode(n->op2);
		if(n->op3 == NULL) { printf("\n"); return; }
		printf(", ");
		eyrePrintNode(n->op3);
		if(n->op4 == NULL) { printf("\n"); return; }
		printf(", ");
		eyrePrintNode(n->op4);
		printf("\n");
	} else if(type == NODE_NAMESPACE) {
		NamespaceNode* n = node;
		printf("namespace %s\n", eyreGetString(n->symbol->base.name)->data);
	} else if(type == NODE_LABEL) {
		LabelNode* n = node;
		printf("%s:\n", eyreGetString(n->symbol->base.name)->data);
	} else if(type == NODE_SYM) {
		SymNode* n = node;
		printf("%s", eyreGetString(n->nameIntern)->data);
	} else if(type == NODE_DOT) {
		DotNode* n = node;
		eyrePrintNode(n->left);
		printf(".");
		printf("%s", eyreGetString(n->right->nameIntern)->data);
	} else if(type == NODE_INVOKE) {
		InvokeNode* n = node;
		eyrePrintNode(n->invoker);
		printf("(");
		for(int i = 0; i < n->argCount; i++) {
			eyrePrintNode(n->args[i]);
			if(i != n->argCount - 1) printf(", ");
		}
		printf(")");
	} else if(type == NODE_SCOPE_END) {
		printf("scope bufferEnd\n");
	} else if(type == NODE_REL) {
		RelNode* n = node;
		printf("rel(");
		eyrePrintNode(n->positive);
		printf(", ");
		eyrePrintNode(n->negative);
		printf(", ");
		eyrePrintNode(n->scalar);
	}

	else {
		error("Invalid node for printing: %d (%s)", type, eyreNodeNames[type]);
	}
}



void eyrePrintNodes(SrcFile* inputSrcFile) {
	for(int i = 0; i < inputSrcFile->nodeCount; i++) {
		printf("Line %d:  ", inputSrcFile->nodeLines[i]);
		eyrePrintNode(inputSrcFile->nodes[i]);
	}
}