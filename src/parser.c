#include "eyre.h"
#include <stdlib.h>
#include <mem.h>



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



#define tokenCapacity 65536

#define stringBuilderCapacity 8192

static SrcFile srcFile;

static u32 tokens[tokenCapacity];

static u8 tokenTypes[tokenCapacity];

static u8 newlines[tokenCapacity >> 3];

static u8 terminators[tokenCapacity >> 3];

static int tokenCount;

static int pos = 0;

static char* chars;

static int size;

static int lineCount;

static char stringBuilder[stringBuilderCapacity];



static void lexerError(char* format, ...) {
	fprintf(stderr, "Lexer error at %s:%d: ", srcFile.path, lineCount);
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	fprintf(stderr, "\n");
	exit(1);
}



static inline void setTerminator() {
	terminators[tokenCount >> 3] &= (1 << (tokenCount & 7));
}



static inline void setNewline() {
	newlines[tokenCount >> 3] &= (1 << (tokenCount & 7));
}



static void addToken(EyreTokenType type, u32 value) {
	if(tokenCount >= tokenCapacity)
		lexerError("Too many tokens: %d", tokenCapacity); // May only happen with a massive file
	tokenTypes[tokenCount] = type;
	tokens[tokenCount] = value;
	tokenCount++;
}



static void addSymbol(EyreTokenSymbol symbol) {
	setTerminator();
	addToken(T_SYM, symbol);
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
			lexerError("Integer out of range");
		number = (number << 1) | c;
	}

	if(number > UINT32_MAX)
		lexerError("64-bit integers not yet supported");

	addToken(T_INT, (u32) number);
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
			eyreLogError("Integer out of range");
		number = (number * 10) + c;
	}

	if(number > UINT32_MAX)
		lexerError("64-bit integers are not yet supported");

	addToken(T_INT, (u32) number);
}



static inline void doubleSymbol(char secondChar, EyreTokenSymbol firstSymbol, EyreTokenSymbol secondSymbol) {
	pos++;
	if(chars[pos] == secondChar) {
		pos++;
		addSymbol(secondSymbol);
	} else {
		addSymbol(firstSymbol);
	}
}



static inline void onNewline() {
	setNewline();
	setTerminator();
	lineCount++;
}



static char escape() {
	char input = chars[pos];
	// Increment pos in this function in case multi-char escapes are implemented in the future
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
		case '\t': return "\\t";
		case '\n': return "\\n";
		case '\r': return "\\r";
		case '\0': return "\\0";
		case '\b': return "\\b";
		default:   return NULL;
	}
}



void eyreLex(SrcFile* inputSrcFile) {
	srcFile = *inputSrcFile;
	chars = srcFile.data;
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

			case '0': {
				char base = chars[++pos];
				if(base == 'b') {
					pos++;
					parseBinary();
				} else if(base == 'd') {
					pos++;
					parseDecimal();
				} else {
					parseDecimal();
				}
				break;
			}

			case '1'...'9': {
				parseDecimal();
				break;
			}

			case '_':
			case 'a'...'z':
			case 'A'...'Z': {
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
				addToken(T_ID, eyreIntern(string, length));
				break;
			}

			case '/': {
				char next = chars[++pos];

				if(next == '/') {
					while(pos < size && chars[++pos] != '\n') { }
					break;
				}

				if(next != '*') {
					addSymbol(S_SLASH);
					break;
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

				break;
			}

			case '\'': {
				pos++;
				char c = chars[pos++];
				if(c == '\\') c = escape();
				addToken(T_CHAR, c);
				if(chars[pos++] != '\'') lexerError("Unterminated char literal");
				break;
			}

			case '"': {
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

				char* string = eyreAllocPersistent(length);
				memcpy(string, stringBuilder, length);
				addToken(T_STRING, eyreIntern(string, length));
				break;
			}

			case '|': doubleSymbol('|', S_PIPE, S_LOGICAL_OR); break;
			case '&': doubleSymbol('&', S_AMPERSAND, S_LOGICAL_AND); break;
			case ':': doubleSymbol(':', S_COLON, S_REFERENCE); break;
			case '!': doubleSymbol('=', S_INEQUALITY, S_EXCLAMATION); break;
			case '=': doubleSymbol('=', S_EQUALITY, S_EQUALS); break;
			case '<': doubleSymbol('=', S_LTE, S_LT); break;
			case '>': doubleSymbol('=', S_GTE, S_GT); break;
			case '+': pos++; addSymbol(S_PLUS); break;
			case '-': pos++; addSymbol(S_MINUS); break;
			case '{': pos++; addSymbol(S_LBRACE); break;
			case '}': pos++; addSymbol(S_RBRACE); break;
			case '(': pos++; addSymbol(S_LPAREN); break;
			case ')': pos++; addSymbol(S_RPAREN); break;
			case '[': pos++; addSymbol(S_LBRACKET); break;
			case ']': pos++; addSymbol(S_RBRACKET); break;
			case ';': pos++; addSymbol(S_SEMICOLON); break;
			case '.': pos++; addSymbol(S_DOT); break;
			case '~': pos++; addSymbol(S_TILDE); break;
			case '*': pos++; addSymbol(S_ASTERISK); break;
			case ',': pos++; addSymbol(S_COMMA); break;

			default: lexerError("Invalid ascii codepoint: %c", chars[pos]);
		}
	}
}



void printTokens() {
	for(int i = 0; i < tokenCount; i++) {
		u8 type = tokenTypes[i];
		u32 value = tokens[i];
		if(type == T_INT) {
			printf("INT   %u\n", value);
		} else if(type == T_ID) {
			Intern* intern = getIntern(value);
			printf("ID    %.*s, (id=%d, hash=%d)\n", intern->length, intern->string, intern->id, intern->hash);
		} else if(type == T_SYM) {
			printf("SYM   %s\n", eyreTokenSymbolNames[value]);
		} else if(type == T_CHAR) {
			char* escaped = reverseEscape((char) value);
			if(escaped != NULL)
				printf("CHAR  '%s' (%d)\n", escaped, value);
			else
				printf("CHAR  '%c' (%d)\n", value, value);
		} else if(type == T_STRING) {
			printf("STR   \"%s\"\n", getIntern(value)->string);
		}
	}
}