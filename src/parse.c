#include "eyre.h"
#include "lex.h"
#include "log.h"
#include "intern.h"
#include <mem.h>
#include "parse.h"
#include "symbol.h"



// Variables



static int pos = 0;

#define nodeCapacity 8192

static void* nodes[nodeCapacity];

static int nodeCount;

static short nodeLines[nodeCapacity];

static char nodeData[16384];

static void* nodeDataEnd = nodeData + 16384;

static void* nodeDataPos = nodeData;

static SrcFile* srcFile;

#define scopesCapacity 32

static int scopeInterns[scopesCapacity];

static int scopes[scopesCapacity];

static int scopeHashes[scopesCapacity];

static int scopesSize = 0;

static int currentScope = 0;

static int currentScopeHash = 0;

static int currentNamespace = 0;



// Error functions



#define parserErrorOffset(format, offset, ...) parserError_(format, offset, __FILE__, __LINE__, ##__VA_ARGS__)

#define parserError(format, ...) parserError_(format, 1, __FILE__, __LINE__, ##__VA_ARGS__)

static void parserError_(char* format, int offset, char* file, int line, ...) {
	fprintf(stderr, "Parser error at %s:%d: ", srcFile->path, tokenLines[pos - offset]);
	va_list args;
	va_start(args, line);
	vfprintf(stderr, format, args);
	fprintf(stderr, "\n");
	eyreError_("Parser error", file, line);
}



// Node functions



static inline void addNode(void* node) {
	if(nodeCount >= nodeCapacity)
		parserError("Too many nodes");
	if(pos >= tokenCount)
		nodeLines[nodeCount] = tokenLines[tokenCount - 1];
	else
		nodeLines[nodeCount] = tokenLines[pos - 1];
	nodes[nodeCount++] = node;
}



static void* createNode(int size, EyreNodeType type) {
	char* node = nodeDataPos;
	*node = type;
	nodeDataPos += (size + 7) & -8; // Align by 8.
	if(nodeDataPos >= nodeDataEnd)
		parserError("Ran out of node data");
	return node;
}



static void* createUnaryNode(char op, void* value) {
	UnaryNode* node = createNode(sizeof(UnaryNode), NODE_UNARY);
	node->type = NODE_UNARY;
	node->op = op;
	node->value = value;
	return node;
}



static void* createBinaryNode(char op, void* left, void* right) {
	BinaryNode* node = createNode(sizeof(BinaryNode), NODE_BINARY);
	node->type = NODE_BINARY;
	node->op = op;
	node->left = left;
	node->right = right;
	return node;
}



static void addScopeEndNode() {
	addNode(createNode(sizeof(ScopeEndNode), NODE_SCOPE_END));
}



// Parsing utils



static int atTerminator() {
	return terminators[pos >> 3] & (1 << (pos & 7));
}

static int parseId() {
	if(tokenTypes[pos] != TOKEN_ID)
		parserError("Expecting identifier");
	return tokens[pos++];
}

static void expectToken(EyreTokenType type) {
	if(tokenTypes[pos++] != type)
		parserError("Expecting token: %s, found: %s", eyreTokenNames[type], eyreTokenNames[pos - 1]);
}

static void expectTerminator() {
	if(!atTerminator())
		parserError("Expecting statement end");
}



// EXPRESSION PARSING



static void* parseExpression(int precedence);



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
	int token = (int) tokens[pos++];

	if(type == TOKEN_ID) {
		int reg = eyreInternToRegister(token);

		if(reg >= 0) {
			RegNode* node = createNode(sizeof(RegNode), NODE_REG);
			node->regType = reg >> 4;
			node->regValue = reg & 15;
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

		if(type == TOKEN_MINUS)
			return createUnaryNode(UNARY_NEG, parseAtom());

		if(type == TOKEN_PLUS)
			return createUnaryNode(UNARY_POS, parseAtom());

		if(type == TOKEN_TILDE)
			return createUnaryNode(UNARY_NOT, parseAtom());

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

			void** args = eyreAllocPersistent(sizeof(void*));
			int argCount = 0;

			while(1) {
				if(tokenTypes[pos] == TOKEN_RPAREN) break;
				eyreAllocPersistentContiguous((void*) &args, argCount * sizeof(void*));
				args[argCount++] = parseExpression(0);
				if(tokenTypes[pos] != TOKEN_COMMA) break;
				pos++;
			}

			expectToken(TOKEN_RPAREN);

			node->args = args;
			node->argCount = argCount;
		} else {
			atom = createBinaryNode(op, atom, parseExpression(precedence2 + 1));
		}
	}

	return atom;
}



// Keyword parsing



static void parseScope();



static int addScope(int intern) {
	if(scopesSize >= scopesCapacity)
		parserError("Too many scopes");
	int hash = scopeHashes[scopesSize - 1] * 31 + intern;
	currentScope = eyreInternScope(scopeInterns, scopesSize, hash);
	scopeHashes[scopesSize] = hash;
	scopes[scopesSize] = currentScope;
	scopeInterns[scopesSize++] = intern;
	return currentScope;
}



static void parseNamespace() {
	int name = parseId();
	int parentScope = currentScope;
	int scope = addScope(name);

	NamespaceSymbol* symbol = (NamespaceSymbol*) eyreAddSymbol(SYM_NAMESPACE, parentScope, name, sizeof(NamespaceSymbol));
	NamespaceNode* node = createNode(sizeof(NamespaceNode), NODE_NAMESPACE);
	node->name = name;
	node->symbol = symbol;
	node->symbol->thisScope = scope;

	if(tokenTypes[pos] == TOKEN_LBRACE) {
		pos++;
		addNode(node);
		parseScope();
		expectToken(TOKEN_RBRACE);
		addScopeEndNode();
	} else {
		expectTerminator();
		if(currentNamespace != 0)
			addScopeEndNode();
		addNode(node);
		currentNamespace = scope;
		currentScope = scope;
	}
}

static void parseEnum(int isBitmask) {}

static void parseStruct() {}

static void parseLabel(int name) {
	LabelSymbol* symbol = eyreAddSymbol(SYM_LABEL, currentScope, name, sizeof(LabelSymbol));
	LabelNode* node = createNode(sizeof(LabelNode), NODE_LABEL);
	node->symbol = symbol;
	addNode(node);
}



static void parseKeyword(EyreKeyword keyword) {
	switch(keyword) {
		case KEYWORD_ENUM: parseEnum(FALSE); break;
		case KEYWORD_BITMASK: parseEnum(TRUE); break;
		case KEYWORD_STRUCT: parseStruct(); break;
		case KEYWORD_NAMESPACE: parseNamespace(); break;
		default: parserError("Invalid keyword: %s", eyreKeywordNames[keyword]);
	}
}



// Instruction parsing



static void* parseOperand() {
	char tokenType = tokens[pos];
	char token = tokens[pos];
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
	node->op1 = NULL;
	node->op2 = NULL;
	node->op3 = NULL;
	node->op4 = NULL;

	if(atTerminator()) return node;

	node->op1 = parseOperand();
	if(tokenTypes[pos] != TOKEN_COMMA) return node;
	pos++;

	node->op2 = parseOperand();
	if(tokenTypes[pos] != TOKEN_COMMA) return node;
	pos++;

	node->op3 = parseOperand();
	if(tokenTypes[pos] != TOKEN_COMMA) return node;
	pos++;

	node->op4 = parseOperand();
	if(!atTerminator()) parserError("Expecting statement end");
	return node;
}



static void parseScope() {
	while(1) {
		u8 type = tokenTypes[pos];
		u32 token = tokens[pos];

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

	currentScope = scopes[--scopesSize];
}



// Public functions



void eyreParse(SrcFile* inputSrcFile) {
	pos = 0;
	nodeCount = 0;
	srcFile = inputSrcFile;

	parseScope();

	if(currentNamespace != 0)
		addScopeEndNode();

	srcFile->nodes = eyreAlloc(nodeCount * 8);
	memcpy(srcFile->nodes, nodes, nodeCount * 8);

	srcFile->nodeLines = eyreAlloc(nodeCount * 2);
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

		if(n->regType == REGISTER_BYTE) {
			printf("%s", eyreByteRegNames[n->regValue]);
		} else if(n->regType == REGISTER_WORD) {
			printf("%s", eyreWordRegNames[n->regValue]);
		} else if(n->regType == REGISTER_DWORD) {
			printf("%s", eyreDWordRegNames[n->regValue]);
		} else if(n->regType == REGISTER_QWORD) {
			printf("%s", eyreQWordRegNames[n->regValue]);
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
		printf("scope end\n");
	} else {
		eyreError("Invalid node for printing: %d (%s)", type, eyreNodeNames[type]);
	}
}



void eyrePrintNodes() {
	for(int i = 0; i < nodeCount; i++) {
		printf("Line %d:  ", nodeLines[i]);
		eyrePrintNode(nodes[i]);
	}
}