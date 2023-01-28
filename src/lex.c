#include "eyre.h"
#include "lex.h"
#include <mem.h>
#include "log.h"
#include "intern.h"



char* eyreTokenSymbols[TOKEN_COUNT] = {
	"","","","","","",
	"+","-","=","==",
	"!=","!","(",")",
	"[","]","{","}",
	";",":","<","<=",
	">",">=",".","/",
	"~","|","&","*",
	"::","&&","||",",",
	"<<", ">>", "^",
};



char* eyreTokenNames[TOKEN_COUNT] = {
	"END","INT","LONG","CHAR","STRING","ID",
	"PLUS","MINUS","EQUALS","EQUALITY",
	"INEQUALITY","EXCLAMATION","LPAREN","RPAREN",
	"LBRACKET","RBRACKET","LBRACE","RBRACE",
	"SEMICOLON","COLON","LT","LTE",
	"GT","GTE","DOT","SLASH",
	"TILDE","PIPE","AMPERSAND","ASTERISK",
	"REFERENCE","LOGICAL_AND","LOGICAL_OR","COMMA",
	"LSHIFT", "RSHIFT", "CARET",
};



// Bitmap of valid identifier chars (a-z, A-Z, 0-9, _)
static u8 idMap[32] = {
	0,0,0,0,0,0,
	0b11111111, // 48
	0b00000001, // 56
	0b11111110, // 64
	0b11111111, // 72
	0b11111111, // 80
	0b10000111, // 88
	0b11111110, // 96
	0b11111111, // 104
	0b11111111, // 112
	0b00000111, // 120
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};




#define STRING_BUILDER_CAPACITY 8192

static SrcFile srcFile;

static int pos = 0;

static char* chars;

static int size;

static char stringBuilder[STRING_BUILDER_CAPACITY];



char tokenTypes[TOKEN_CAPACITY];

int tokens[TOKEN_CAPACITY];

short tokenLines[TOKEN_CAPACITY];

u8 newlines[TOKEN_CAPACITY >> 3];

u8 terminators[TOKEN_CAPACITY >> 3];

int tokenCount;

int lineCount;



#define lexerError(format, ...) lexerError_(format, __FILE__, __LINE__, ##__VA_ARGS__)

static void lexerError_(char* format, char* file, int line, ...) {
	fprintf(stderr, "Lexer error at %s:%d: ", srcFile.path, lineCount);
	va_list args;
	va_start(args, line);
	vfprintf(stderr, format, args);
	fprintf(stderr, "\n");
	eyreError_("Lexer error", file, line);
}



static inline int isIdChar(char c) {
	return idMap[c >> 3] & (1 << (c & 7));
}



static inline void setTerminator() {
	terminators[tokenCount >> 3] |= (1 << (tokenCount & 7));
}



static inline void setNewline() {
	newlines[tokenCount >> 3] &= (1 << (tokenCount & 7));
}



static void addToken(EyreTokenType type, int value) {
	if(tokenCount >= TOKEN_CAPACITY)
		lexerError("Too many tokens: %d", TOKEN_CAPACITY);
	tokenTypes[tokenCount] = type;
	tokens[tokenCount] = value;
	tokenLines[tokenCount] = lineCount;
	tokenCount++;
}



static void addSymbol(EyreTokenType symbol) {
	setTerminator();
	addToken(symbol, 0);
}



static void parseBinary() {
	u64 number = 0;

	while(1) {
		u8 c = chars[pos];

		if(c == '_') {
			pos++;
			continue;
		}

		c -= '0';
		if(c > 1) break;
		pos++;

		if(number & (1LL << 63))
			lexerError("Integer literal out of range");

		number = (number << 1) | c;
	}

	if(number > UINT32_MAX)
		lexerError("64-bit integers not yet supported");

	if(isIdChar(chars[pos]))
		lexerError("Invalid number char: %d ('%c')", chars[pos], chars[pos]);

	addToken(TOKEN_INT, (int) number);
}



static void parseHex() {
	u64 value = 0;

	while(1) {
		u8 c = chars[pos];

		if(c == '_') {
			pos++;
			continue;
		}

		if(c >= '0' && c <= '9')
			c -= '0';
		else if(c >= 'A' && c <= 'F')
			c -= 'A' - 10;
		else if(c >= 'a' && c <= 'f')
			c -= 'a' - 10;
		else
			break;

		pos++;

		if((value & (0b1111LL << 60)) != 0L)
			lexerError("Integer literal out of range");

		value = (value << 4) | c;
	}

	if(value > UINT32_MAX)
		lexerError("64-bit integers are not yet supported");

	if(isIdChar(chars[pos]))
		lexerError("Invalid value char: %d ('%c')", chars[pos], chars[pos]);

	addToken(TOKEN_INT, (int) value);
}



static void parseDecimal() {
	u64 number = 0;

	while(1) {
		u8 c = chars[pos];

		if(c == '_') {
			pos++;
			continue;
		}

		c -= '0';
		if(c > 9) break;
		pos++;

		if(number & (0xFFLL << 56))
			lexerError("Integer literal out of range");

		number = (number * 10) + c;
	}

	if(number > UINT32_MAX)
		lexerError("64-bit integers are not yet supported");

	if(isIdChar(chars[pos]))
		lexerError("Invalid number char: %d ('%c')", chars[pos], chars[pos]);

	addToken(TOKEN_INT, (int) number);
}



static inline void doubleSymbol(char secondChar, EyreTokenType firstSymbol, EyreTokenType secondSymbol) {
	pos++;
	if(chars[pos] == secondChar) {
		pos++;
		addSymbol(secondSymbol);
	} else {
		addSymbol(firstSymbol);
	}
}



static inline void doubleSymbol2(char char1, int sym1, char char2, int sym2, int symDefault) {
	pos++;
	if(chars[pos] == char1) {
		pos++;
		addSymbol(sym1);
	} else if(chars[pos] == char2) {
		pos++;
		addSymbol(sym2);
	} else {
		addSymbol(symDefault);
	}
}



static inline void onNewline() {
	setNewline();
	setTerminator();
	lineCount++;
}



static char escape() {
	// Increment pos in this function in case multi-char escapes are implemented in the future
	char input = chars[pos];
	switch(input) {
		case 't'  : pos++; return '\t';
		case 'n'  : pos++; return '\n';
		case '\\' : pos++; return '\\';
		case 'r'  : pos++; return '\r';
		case '0'  : pos++; return '\0';
		case 'b'  : pos++; return '\b';
		case '"'  : pos++; return '\"';
		case '\'' : pos++; return '\'';
		default   : lexerError("Invalid escape char: %d ('%c')", input, input); return 0;
	}
}



static char* reverseEscape(char escape) {
	switch(escape) {
		case '\t' : return "\\t";
		case '\n' : return "\\n";
		case '\r' : return "\\r";
		case '\0' : return "\\0";
		case '\b' : return "\\b";
		default   : return NULL;
	}
}



static void readId() {
	char* string = &chars[pos];
	int length = 0;
	while(1) {
		char c = string[length];
		if(idMap[c >> 3] & (1 << (c & 7)))
			length++;
		else
			break;
	}
	pos += length;
	addToken(TOKEN_ID, eyreInternString(string, length));
}



static void readString() {
	pos++;
	int length = 0;

	while(1) {
		char c = chars[pos++];
		if(c == '"') break;
		switch(c) {
			case 0    : lexerError("Unterminated string literal"); break;
			case '\n' : lexerError("Newline not allowed in string literal"); break;
			case '\\' : stringBuilder[length++] = escape(); break;
			default   : stringBuilder[length++] = c; break;
		}
	}

	addToken(TOKEN_STRING, eyreInternString(stringBuilder, length));
}



static void readChar() {
	pos++;
	char c = chars[pos++];
	if(c == '\\') c = escape();
	addToken(TOKEN_CHAR, c);
	if(chars[pos++] != '\'')
		lexerError("Unterminated char literal");
}



static void processSlash() {
	char next = chars[++pos];

	if(next == '/') {
		while(pos < size && chars[++pos] != '\n') { }
		return;
	}

	if(next != '*') {
		addSymbol(TOKEN_SLASH);
		return;
	}

	int count = 1;

	while(count > 0) {
		if(pos >= size)
			lexerError("Unterminated multiline comment");

		char c = chars[pos++];
		if(c == '/' && chars[pos] == '*') {
			count++;
			pos++;
		} else if(c == '*' && chars[pos] == '/') {
			count--;
			pos++;
		} else if(c == '\n') {
			onNewline();
		}
	}
}



static void processZero() {
	switch(chars[++pos]) {
		case 'b': pos++; parseBinary(); break;
		case 'd': pos++; parseDecimal(); break;
		case 'x': pos++; parseHex(); break;
		default: parseDecimal(); break;
	}
}



void eyreLex(SrcFile* inputSrcFile) {
	srcFile = *inputSrcFile;
	chars = srcFile.chars;
	pos = 0;
	size = srcFile.size;
	tokenCount = 0;
	lineCount = 1;

	while(pos < size) {
		switch(chars[pos]) {
			case '\n': pos++; onNewline(); break;

			case '\t':
			case '\r':
			case ' ' : pos++; break;

			case '0': processZero(); break;
			case '1'...'9': parseDecimal(); break;
			case '/': processSlash(); break;
			case '\'': readChar(); break;
			case '"': readString(); break;

			case '_':
			case 'a'...'z':
			case 'A'...'Z': readId(); break;

			case '|': doubleSymbol('|', TOKEN_PIPE, TOKEN_LOGICAL_OR); break;
			case '&': doubleSymbol('&', TOKEN_AMPERSAND, TOKEN_LOGICAL_AND); break;
			case ':': doubleSymbol(':', TOKEN_COLON, TOKEN_REFERENCE); break;
			case '!': doubleSymbol('=', TOKEN_INEQUALITY, TOKEN_EXCLAMATION); break;
			case '=': doubleSymbol('=', TOKEN_EQUALITY, TOKEN_EQUALS); break;
			case '<': doubleSymbol2('<', TOKEN_LSHIFT, '=', TOKEN_LTE, TOKEN_LT); break;
			case '>': doubleSymbol2('>', TOKEN_RSHIFT, '=', TOKEN_GTE, TOKEN_GT); break;
			case '^': pos++; addSymbol(TOKEN_CARET); break;
			case '+': pos++; addSymbol(TOKEN_PLUS); break;
			case '-': pos++; addSymbol(TOKEN_MINUS); break;
			case '{': pos++; addSymbol(TOKEN_LBRACE); break;
			case '}': pos++; addSymbol(TOKEN_RBRACE); break;
			case '(': pos++; addSymbol(TOKEN_LPAREN); break;
			case ')': pos++; addSymbol(TOKEN_RPAREN); break;
			case '[': pos++; addSymbol(TOKEN_LBRACKET); break;
			case ']': pos++; addSymbol(TOKEN_RBRACKET); break;
			case ';': pos++; addSymbol(TOKEN_SEMICOLON); break;
			case '.': pos++; addSymbol(TOKEN_DOT); break;
			case '~': pos++; addSymbol(TOKEN_TILDE); break;
			case '*': pos++; addSymbol(TOKEN_ASTERISK); break;
			case ',': pos++; addSymbol(TOKEN_COMMA); break;

			default: lexerError("Invalid ascii codepoint: %c", chars[pos]);
		}
	}

	// Pad end with EOF tokens
	for(int i = 0; i < 4; i++)
		setTerminator();
}



void eyrePrintTokens() {
	for(int i = 0; i < tokenCount; i++) {
		u8 type = tokenTypes[i];
		u32 value = tokens[i];
		if(type == TOKEN_INT) {
			printf("INT   %u\n", value);
		} else if(type == TOKEN_ID) {
			StringIntern* intern = eyreGetString(value);
			printf("ID    %.*s, (id=%d, hash=%d)\n", intern->length, intern->data, value, intern->hash);
		} else if(type == TOKEN_CHAR) {
			char* escaped = reverseEscape((char) value);
			if(escaped != NULL)
				printf("CHAR  '%s' (%d)\n", escaped, value);
			else
				printf("CHAR  '%c' (%d)\n", value, value);
		} else if(type == TOKEN_STRING) {
			printf("STR   \"%s\"\n", eyreGetString(value)->data);
		} else if(type >= TOKEN_SYM_START) {
			printf("SYM   %s\n", eyreTokenSymbols[type]);
		}
	}
}