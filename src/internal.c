#include "eyre.h"

#define tokenCapacity 66536

extern int tokenValues[tokenCapacity];

extern char tokenTypes[tokenCapacity];

extern short tokenLineNumbers[tokenCapacity];

extern u8 newlines[tokenCapacity / 8];

extern u8 terminators[tokenCapacity / 8];

extern int tokenCount;

extern int lineCount;
