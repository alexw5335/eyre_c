#include "eyre.h"
#include "log.h"


typedef enum {
	N_BINARY,
	N_UNARY,
	N_INT,
	N_STRING,
	N_NAMESPACE,
	N_INSTRUCTION,
	N_SCOPE_END, // no contents
	N_COUNT
} EyreNodeType;



typedef enum {
	BINARY_ADD,
	BINARY_SUB,
	BINARY_MUL,
	BINARY_DIV,
	BINARY_OR,
	BINARY_AND,
	BINARY_XOR,
} BinaryOp;



typedef enum {
	UNARY_PLUS,
	UNARY_MINUS,
	UNARY_NOT,
} UnaryOp;



static SrcFile srcFile;

static u8* tokenTypes;

static u32* tokens;

static int tokenCount;

static int pos = 0;



#define nodeCapacity 16384

static u8 nodeTypes[nodeCapacity];

static u32 nodes[nodeCapacity];

static int nodeCount;



void parseKeyword(EyreKeyword keyword) {

}



void eyreParse(SrcFile* inputSrcFile) {
	srcFile    = *inputSrcFile;
	tokenTypes = srcFile.tokenTypes;
	tokens     = srcFile.tokens;
	tokenCount = srcFile.tokenCount;
	pos        = 0;
	nodeCount  = 0;

	while(1) {
		u8 type = tokenTypes[pos];
		u32 token = tokens[pos];

		if(type == T_END) break;

		switch(type) {
			case T_ID: {
				u32 keyword = token - eyreKeywordInternStart;
				if(keyword < KEYWORD_COUNT)
					parseKeyword(keyword);
			}
		}
	}
}