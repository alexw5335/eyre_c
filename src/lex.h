#ifndef EYRE_LEX_INCLUDE
#define EYRE_LEX_INCLUDE



#define TOKEN_CAPACITY 66536

extern char tokenTypes[TOKEN_CAPACITY];

extern int tokens[TOKEN_CAPACITY];

extern short tokenLines[TOKEN_CAPACITY]; // line numbers of each token

extern u8 newlines[TOKEN_CAPACITY >> 3]; // tokens after which a newline occurs

extern u8 terminators[TOKEN_CAPACITY >> 3]; // tokens after which a newline, eof, or symbol occurs

extern int tokenCount;

extern int lineCount;



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



extern char* eyreTokenSymbols[TOKEN_COUNT];

extern char* eyreTokenNames[TOKEN_COUNT];

#define TOKEN_SYM_START TOKEN_PLUS



#endif