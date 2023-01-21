#ifndef EYRE_LEX_INCLUDE
#define EYRE_LEX_INCLUDE



#define TOKEN_CAPACITY 66536

extern char tokenTypes[TOKEN_CAPACITY];

extern int tokens[TOKEN_CAPACITY];

extern u8 newlines[TOKEN_CAPACITY >> 3];

extern u8 terminators[TOKEN_CAPACITY >> 3];

extern int tokenCount;

extern int lineCount;



#endif