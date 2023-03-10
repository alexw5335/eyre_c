#ifndef INCLUDE_ENUMS
#define INCLUDE_ENUMS



typedef enum EyreTokenType {
	TOKEN_END,
	TOKEN_INT,
	TOKEN_LONG,
	TOKEN_CHAR,
	TOKEN_STRING,
	TOKEN_ID,
	TOKEN_PLUS,
	TOKEN_MINUS,
	TOKEN_EQUALS,
	TOKEN_EQUALITY,
	TOKEN_INEQUALITY,
	TOKEN_EXCLAMATION,
	TOKEN_LPAREN,
	TOKEN_RPAREN,
	TOKEN_LBRACKET,
	TOKEN_RBRACKET,
	TOKEN_LBRACE,
	TOKEN_RBRACE,
	TOKEN_SEMICOLON,
	TOKEN_COLON,
	TOKEN_LT,
	TOKEN_LTE,
	TOKEN_GT,
	TOKEN_GTE,
	TOKEN_DOT,
	TOKEN_SLASH,
	TOKEN_TILDE,
	TOKEN_PIPE,
	TOKEN_AMPERSAND,
	TOKEN_ASTERISK,
	TOKEN_REFERENCE,
	TOKEN_LOGICAL_AND,
	TOKEN_LOGICAL_OR,
	TOKEN_COMMA,
	TOKEN_LSHIFT,
	TOKEN_RSHIFT,
	TOKEN_CARET,
	TOKEN_COUNT
} EyreTokenType;



#define TOKEN_SYM_START TOKEN_PLUS



typedef enum {
	NODE_REG,
	NODE_INT,
	NODE_SYM,
	NODE_BINARY,
	NODE_UNARY,
	NODE_MEM,
	NODE_IMM,
	NODE_INS,
	NODE_STRUCT,
	NODE_SCOPE_END,
	NODE_NAMESPACE,
	NODE_ENUM,
	NODE_LABEL,
	NODE_DOT,
	NODE_INVOKE,
	NODE_REL,
	NODE_COUNT
} EyreNodeType;



typedef enum EyreUnaryOp {
	UNARY_POS,
	UNARY_NEG,
	UNARY_NOT,
	UNARY_COUNT
} EyreUnaryOp;



typedef enum EyreBinaryOp {
	BINARY_ADD,
	BINARY_SUB,
	BINARY_MUL,
	BINARY_DIV,
	BINARY_AND,
	BINARY_OR,
	BINARY_XOR,
	BINARY_SHL,
	BINARY_SHR,
	BINARY_DOT,
	BINARY_INV,
	BINARY_COUNT
} EyreBinaryOp;



typedef enum {
	REG_SPL,
	REG_BPL,
	REG_SIL,
	REG_DIL,

	REG_AL,
	REG_CL,
	REG_DL,
	REG_BL,
	REG_AH,
	REG_CH,
	REG_DH,
	REG_BH,
	REG_R8B,
	REG_R9B,
	REG_R10B,
	REG_R11B,
	REG_R12B,
	REG_R13B,
	REG_R14B,
	REG_R15B,

	REG_AX,
	REG_CX,
	REG_DX,
	REG_BX,
	REG_SP,
	REG_BP,
	REG_SI,
	REG_DI,
	REG_R8W,
	REG_R9W,
	REG_R10W,
	REG_R11W,
	REG_R12W,
	REG_R13W,
	REG_R14W,
	REG_R15W,

	REG_EAX,
	REG_ECX,
	REG_EDX,
	REG_EBX,
	REG_ESP,
	REG_EBP,
	REG_ESI,
	REG_EDI,
	REG_R8D,
	REG_R9D,
	REG_R10D,
	REG_R11D,
	REG_R12D,
	REG_R13D,
	REG_R14D,
	REG_R15D,

	REG_RAX,
	REG_RCX,
	REG_RDX,
	REG_RBX,
	REG_RSP,
	REG_RBP,
	REG_RSI,
	REG_RDI,
	REG_R8,
	REG_R9,
	REG_R10,
	REG_R11,
	REG_R12,
	REG_R13,
	REG_R14,
	REG_R15,
} EyreRegister;



typedef enum {
	KEYWORD_CONST,
	KEYWORD_STRUCT,
	KEYWORD_NAMESPACE,
	KEYWORD_PROC,
	KEYWORD_ENUM,
	KEYWORD_BITMASK,
	KEYWORD_DLLIMPORT,
	KEYWORD_VAR,
	KEYWORD_COUNT,
} EyreKeyword;



typedef enum {
	WIDTH_BYTE,
	WIDTH_WORD,
	WIDTH_DWORD,
	WIDTH_QWORD,
	WIDTH_COUNT,
} EyreWidth;



typedef enum {
	VARWIDTH_DB,
	VARWIDTH_DW,
	VARWIDTH_DD,
	VARWIDTH_DQ,
	VARWIDTH_COUNT,
} EyreVarWidth;



typedef enum {
	SPECIFIER_NONE,
	SPECIFIER_O,
	SPECIFIER_RM_I8,
	SPECIFIER_RM_1,
	SPECIFIER_RM_CL,
	SPECIFIER_A_I,
	SPECIFIER_COUNT,
} EyreSpecifier;



typedef enum {
	OPERANDS_NONE,
	OPERANDS_R,
	OPERANDS_M,
	OPERANDS_O,
	OPERANDS_R_R,
	OPERANDS_R_M,
	OPERANDS_M_R,
	OPERANDS_R_I,
	OPERANDS_M_I,
	OPERANDS_R_I8,
	OPERANDS_M_I8,
	OPERANDS_A_I,
	OPERANDS_RM_1,
	OPERANDS_RM_CL,
	OPERANDS_CUSTOM1,
	OPERANDS_CUSTOM2,
	OPERANDS_COUNT,
} EyreOperands;



typedef enum {
	COMPOUND_RM,
	COMPOUND_RM_R,
	COMPOUND_R_RM,
	COMPOUND_RM_I,
	COMPOUND_RM_I8,
	COMPOUND_COUNT,
} EyreCompoundOperands;



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



typedef enum {
	// Invalid
	SECTION_NONE,
	// .text, initialised | code, execute | read
	SECTION_TEXT,
	// .data, initialised, read | write
	SECTION_DATA,
	// .idata, initialised, read
	SECTION_IDATA,
	SECTION_COUNT,
} EyreSection;



extern char* eyreWidthNames[WIDTH_COUNT];

extern char* eyreVarWidthNames[VARWIDTH_COUNT];

extern char* eyreOperandsNames[OPERANDS_COUNT];

extern int eyreOperandsSpecifiers[OPERANDS_COUNT];

extern char* eyreCompoundOperandsNames[COMPOUND_COUNT];

extern EyreOperands eyreCompoundOperandsMap[COMPOUND_COUNT][3];

extern char* eyreNodeNames[NODE_COUNT];

extern char* eyreUnaryOpSymbols[UNARY_COUNT];

extern char* eyreBinaryOpSymbols[BINARY_COUNT];

extern char* eyreByteRegNames[16];

extern char* eyreWordRegNames[16];

extern char* eyreDWordRegNames[16];

extern char* eyreQWordRegNames[16];

extern char* eyreKeywordNames[KEYWORD_COUNT];

extern char* eyreSpecifierNames[SPECIFIER_COUNT];

extern char* eyreTokenSymbols[TOKEN_COUNT];

extern char* eyreTokenNames[TOKEN_COUNT];



int calcUnaryInt(EyreUnaryOp op, int value);

int calcBinaryInt(EyreBinaryOp op, int left, int right);


#endif
