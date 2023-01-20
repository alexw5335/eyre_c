#include "eyre.h"
#include "log.h"



static SrcFile srcFile;

static u8* tokenTypes;

static u32* tokens;

static int tokenCount;

static int pos = 0;

static u8* terminators;



static int atTerminator() {
	return terminators[pos >> 3] & (1 << (pos & 7));
}



static char nodeTypes[66536];

static void* nodes[66536];

static int nodePos;

static void* nodeBank[66536];

static void* nodeBankPos;



void intNode(int value) {
	void* pointer = nodeBankPos;
}



/*static void parseAtom() {
	EyreTokenType type = tokenTypes[pos];
	int token = (int) tokens[pos++];

	if(type == TOKEN_ID) {
		int reg = (int) token - eyreRegisterInternStart;

		if(reg < eyreRegisterInternCount) {
			addNode(NODE_REG, reg);
			return;
		}

		addNode(NODE_SYM, token);
		return;
	}

	if(type == TOKEN_INT) {
		addNode(NODE_INT, token);
		return;
	}

	if(type > TOKEN_SYM_START) {
		if(type == TOKEN_MINUS)
			addNode(NODE_NEG, 0);
		else if(type == TOKEN_PLUS)
			addNode(NODE_POS, 0);
		else if(type == TOKEN_TILDE)
			addNode(NODE_NOT, 0);
		else
			eyreLogError("Invalid atom token (type = %s, token = %d)", eyreTokenNames[type], token);
	}

	eyreLogError("Invalid atom token (type = %s, token = %d)", eyreTokenNames[type], token);
}




static int opPrecedence(EyreTokenType op) {
	switch(op) {
		case TOKEN_LPAREN: return 6;
		case TOKEN_DOT: return 5;

		case TOKEN_ASTERISK:
		case TOKEN_SLASH: return 4;

		case TOKEN_PLUS:
		case TOKEN_MINUS: return 3;

		case TOKEN_LSHIFT:
		case TOKEN_RSHIFT: return 2;

		case TOKEN_AMPERSAND:
		case TOKEN_CARET:
		case TOKEN_PIPE: return 1;
		default: return 0;
	}
}



static void parseExpression(int precedence) {
	parseAtom();

	while(1) {
		EyreTokenType type = tokenTypes[pos];
		int token = (int) tokens[pos];

		if(type < TOKEN_SYM_START) {
			if(!atTerminator()) {
				eyreLogError("Use a semicolon to separate expressions that are on the same line");
			} else {
				break;
			}
		}

		if(type == TOKEN_SEMICOLON) break;
		int precedence2 = opPrecedence(type);
		pos++;

		parseAtom();
	}
}



static void parseKeyword(EyreKeyword keyword) {
	pos++;
}



static void parseInstruction(EyreMnemonic mnemonic) {
	pos++;

}



void eyreParse(SrcFile* inputSrcFile) {
	srcFile    = *inputSrcFile;
	tokenTypes = srcFile.tokenTypes;
	tokens     = srcFile.tokens;
	tokenCount = srcFile.tokenCount;
	pos        = 0;
	nodeCount  = 0;
	terminators = srcFile.terminators;

	while(1) {
		u8 type = tokenTypes[pos];
		u32 token = tokens[pos];

		if(type == TOKEN_END) break;

		if(type == TOKEN_ID) {
			u32 keyword = token - eyreKeywordInternStart;

			if(keyword < KEYWORD_COUNT)
				parseKeyword(keyword);

			u32 mnemonic = token - eyreMnemonicInternStart;

			if(mnemonic < MNEMONIC_COUNT)
				parseInstruction(mnemonic);

			eyreLogError("Invalid token (type = %s, token = %d)", eyreTokenNames[type], token);
		}

	}
}*/