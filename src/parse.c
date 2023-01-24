#include "eyre.h"
#include "lex.h"
#include "log.h"



static int pos = 0;

#define NODE_CAPACITY 8192

static void* nodes[NODE_CAPACITY];

static int nodeCount;

static char nodeData[16384];

static void* nodeDataEnd = nodeData + 16384;

static void* nodeDataPos = nodeData;

static SrcFile* srcFile;



#define parserErrorOffset(format, offset, ...) parserError_(format, offset, __FILE__, __LINE__, ##__VA_ARGS__)

#define parserError(format, ...) parserError_(format, 0, __FILE__, __LINE__, ##__VA_ARGS__)

static void parserError_(char* format, int offset, char* file, int line, ...) {
	fprintf(stderr, "Parser error at %s:%d: ", srcFile->path, tokenLines[pos - offset]);
	va_list args;
	va_start(args, line);
	vfprintf(stderr, format, args);
	fprintf(stderr, "\n");
	eyreError("Parser error", file, line);
}



static int atTerminator() {
	return terminators[pos >> 3] & (1 << (pos & 7));
}



static inline void addNode(void* node) {
	if(nodeCount >= NODE_CAPACITY)
		parserError("Too many nodes");
	nodes[nodeCount++] = node;
}



static inline void* getNodeData() {
	return nodeDataPos;
}



static inline void* createNode(int size) {
	void* node = nodeDataPos;
	nodeDataPos += (size + 7) & -8; // Align by 8.
	if(nodeDataPos >= nodeDataEnd)
		parserError("Ran out of node data");
	return node;
}



static inline void* addNewNode(int size) {
	void* node = createNode(size);
	addNode(node);
	return node;
}



static void* createIntNode(int value) {
	IntNode* node = createNode(sizeof(IntNode));
	node->type = NODE_INT;
	node->value = value;
	return node;
}



static void* createRegNode(char regType, char regValue) {
	RegNode* node = createNode(sizeof(RegNode));
	node->type = NODE_REG;
	node->regType = regType;
	node->regValue = regValue;
	return node;
}



static void* createSymNode(int nameIntern) {
	SymNode* node = createNode(sizeof(SymNode));
	node->type = NODE_SYM;
	node->nameIntern = nameIntern;
	return node;
}



static void* createUnaryNode(char op, void* value) {
	UnaryNode* node = createNode(sizeof(UnaryNode));
	node->type = NODE_UNARY;
	node->op = op;
	node->value = value;
	return node;
}




static void* createBinaryNode(char op, void* left, void* right) {
	BinaryNode* node = createNode(sizeof(BinaryNode));
	node->type = NODE_BINARY;
	node->op = op;
	node->left = left;
	node->right = right;
	return node;
}



static void* createMemNode(char width, void* value) {
	MemNode* node = createNode(sizeof(MemNode));
	node->type = NODE_MEM;
	node->width = width;
	node->value = value;
	return node;
}



static void* createImmNode(void* value) {
	ImmNode* node = createNode(sizeof(ImmNode));
	node->type = NODE_IMM;
	node->value = value;
	return node;
}



static int opPrecedence(char op) {
	switch(op) {
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
		default:              return -1;
	}
}



static void* parseAtom();

static void* parseExpression(int precedence);



static void* parseAtom() {
	char type = tokenTypes[pos];
	int token = (int) tokens[pos++];

	if(type == TOKEN_ID) {
		int reg = (int) token - eyreRegisterInternStart;

		if(reg < eyreRegisterInternCount) {
			int regType = reg >> 4;
			int regValue = reg & 15;
			return createRegNode((char) regType, (char) regValue);
		}

		return createSymNode(token);
	}

	if(type == TOKEN_INT)
		return createIntNode(token);

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

		atom = createBinaryNode(op, atom, parseExpression(precedence2 + 1));
	}

	return atom;
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



static void parseNamespace() {
	int name = parseId();
	if(tokenTypes[pos] == TOKEN_LBRACE) {
		expectTerminator();
	} else {
		pos++;
	}
}



static void parseEnum(int isBitmask) {
	int enumName = parseId();

	expectToken(TOKEN_LBRACE);

/*	EnumEntryNode* entries = getNodeData();

	while(TRUE) {
		if(tokenTypes[pos] == TOKEN_RBRACE)
			break;

		int name = parseId();

		if()
	}*/
	expectToken(TOKEN_RBRACE);

	EnumNode* enumNode = addNewNode(sizeof(EnumNode));
	enumNode->type = NODE_ENUM;
	enumNode->name = enumName;
}



static void parseStruct() {
	int name = parseId();
	expectToken(TOKEN_LBRACE);
	expectToken(TOKEN_RBRACE);
	StructNode* node = addNewNode(sizeof(StructNode));
	node->name = name;
	node->type = NODE_STRUCT;
	expectTerminator();
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



static void* parseOperand() {
	char tokenType = tokens[pos];
	char token = tokens[pos];
	int width = -1;

	if(tokenType == TOKEN_ID) {
		width = token - eyreWidthInternStart;
		if(width >= eyreWidthInternCount)
			width = -1;
		if(tokenTypes[pos + 1] == TOKEN_LBRACKET)
			tokenType = tokenTypes[++pos];
	}

	if(tokenType == TOKEN_LBRACKET) {
		pos++;
		void* value = parseExpression(0);
		if(tokenTypes[pos++] != TOKEN_RBRACKET)
			parserError("Expecting ']'");
		return createMemNode(width, value);
	}

	void* node = parseExpression(0);
	char nodeType = *(char*) node;
	if(nodeType == NODE_REG)
		return node;
	else
		return createImmNode(node);
}



static void* parseInstruction(EyreMnemonic mnemonic) {
	InsNode* node = createNode(sizeof(InsNode));
	node->type = NODE_INS;
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



void eyreParse(SrcFile* inputSrcFile) {
	pos = 0;
	nodeCount = 0;
	srcFile = inputSrcFile;

	while(1) {
		u8 type = tokenTypes[pos];
		u32 token = tokens[pos];

		if(type == TOKEN_END) break;
		if(type == TOKEN_ID) {
			u32 keyword = token - eyreKeywordInternStart;

			if(keyword < KEYWORD_COUNT) {
				pos++;
				parseKeyword(keyword);
				continue;
			}

			u32 mnemonic = token - eyreMnemonicInternStart;

			if(mnemonic < MNEMONIC_COUNT) {
				pos++;
				addNode(parseInstruction(mnemonic));
				continue;
			}

			parserError("Invalid token (type = %s, token = %d)", eyreTokenNames[type], token);
		}

		parserError("Invalid token (type = %s, token = %d)", eyreTokenNames[type], token);
	}
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
		if(n->op1 == NULL) return;
		printf(" ");
		eyrePrintNode(n->op1);
		if(n->op2 == NULL) return;
		printf(", ");
		eyrePrintNode(n->op2);
		if(n->op3 == NULL) return;
		printf(", ");
		eyrePrintNode(n->op3);
		if(n->op4 == NULL) return;
		printf(", ");
		eyrePrintNode(n->op4);
	} else {
		parserError("Invalid node for printing: %d", eyreNodeNames[type]);
	}
}



void eyrePrintNodes() {
	for(int i = 0; i < nodeCount; i++)
		eyrePrintNode(nodes[i]);
}