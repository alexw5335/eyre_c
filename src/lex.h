#include "eyre.h"

#define TOKEN_CAPACITY 66536

static char tokenTypes[TOKEN_CAPACITY];

static int tokens[TOKEN_CAPACITY];

static char newlines[TOKEN_CAPACITY >> 3];

static char terminators[TOKEN_CAPACITY >> 3];

static int tokenCount;

static int lineCount;