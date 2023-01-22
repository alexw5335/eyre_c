#include <fileapi.h>
#include <handleapi.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <processenv.h>








static char stringBuilder[512];

static char* chars;

static int size;

static int pos;



static void println(char* format, ...) {
	va_list list;
	va_start(list, format);
	int length = vsprintf(stringBuilder, format, list);
	stringBuilder[length++] = '\n';
	stringBuilder[length++] = 0;
	printf("%s", stringBuilder);
}



static void error(char* format, ...) {
	va_list list;
	va_start(list, format);
	vfprintf(stderr, format, list);
	fprintf(stderr, "\n");
	exit(1);
}



static void readFile(char* path) {
	HANDLE handle = CreateFileA(
		path,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if(handle == INVALID_HANDLE_VALUE)
		error("Error opening file: %s", path);

	size = (int) GetFileSize(handle, NULL);
	chars = malloc(size);

	unsigned long numBytesRead;
	int readResult = ReadFile(handle, chars, size, &numBytesRead, NULL);

	if(readResult == 0)
		error("error reading file: %s", path);
}



static char* getLocalFile(char* fileName) {
	int fileNameLength = (int) strlen(fileName);
	int length = (int) GetCurrentDirectoryA(0, NULL);
	char* file = malloc(length + fileNameLength);
	GetCurrentDirectoryA(length, file);

	int index;
	for(int i = length - 1; i >= 0; i--) {
		if(file[i] == '\\') {
			index = i + 1;
			break;
		}
	}

	for(int i = 0; i < fileNameLength; i++)
		file[index + i] = fileName[i];

	file[index + fileNameLength] = 0;

	return file;
}



static int parseHex(unsigned char c) {
	if(c >= '0' && c <= '9')
		return c - '0';
	if(c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if(c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	error("Invalid hex: %d", c);
	return 0;
}



static int parseHexString(char* string, int length) {
	int value = 0;
	for(int i = 0; i < length; i++)
		value = (value << 4) | parseHex(string[i]);
	return value;
}



typedef unsigned long long u64;



u64 makeAscii8(char* string, int length) {
	if(length > 7)
		error("String too long: %.*s", length, string);

	u64 value = 0;

	for(int i = length - 1; i >= 0; i--)
		value = (value << 8) | string[i];

	return value;
}



u64 parseAscii8() {
	char* string = &chars[pos];
	int length = 0;
	while(string[length] != ' ' && string[length] != '\n') length++;
	pos += length;
	return makeAscii8(string, length);
}



typedef struct {
	char chars[16];
} Mnemonic;



typedef enum {
	OPERANDS_NONE,
	OPERANDS_R,
	OPERANDS_M,
	OPERANDS_R_R,
	OPERANDS_R_M,
	OPERANDS_M_R,
	OPERANDS_R_I,
	OPERANDS_M_I,
	OPERANDS_R_I8,
	OPERANDS_M_I8,
	OPERANDS_COUNT,
} Operands;



typedef enum {
	OPERANDS2_RM_R,
	OPERANDS2_R_RM,
	OPERANDS2_RM_I,
	OPERANDS2_RM_I8,
	OPERANDS2_COUNT,
} CompoundOperands;

typedef enum {
	WIDTHS_NO8,
} Widths;

char* operandsNames[OPERANDS_COUNT] = {
	"NONE", "R", "M", "R_R", "R_M", "M_R"
};

char* compoundOperandsNames[OPERANDS2_COUNT] = {
	"RM_R", "R_RM", "RM_I", "RM_I8"
};

u64 operandsAscii8[OPERANDS_COUNT];

u64 compoundOperandsAscii8[OPERANDS2_COUNT];

#define operandsBit(operands) (1 << operands)

void init() {
	for(int i = 0; i < OPERANDS2_COUNT; i++) {
		char* name = compoundOperandsNames[i];
		compoundOperandsAscii8[i] = makeAscii8(name, strlen(name));
	}
}



typedef struct {
	int      mnemonic;
	int      opcode;
	int      extension;
	Operands operands;
	Widths   widths;
} Encoding;



typedef struct {
	int   mnemonic;
	int   operandsBits;
	int   specificBits;
	short encodings[32];
} EncodingGroup;



#define ENCODINGS_CAPACITY 1024
#define MNEMONICS_CAPACITY 512
#define GROUPS_CAPACITY 512

int mnemonicsSize;
Mnemonic mnemonics[MNEMONICS_CAPACITY];

int encodingsSize;
Encoding encodings[ENCODINGS_CAPACITY];

int groupsSize;
EncodingGroup groups[GROUPS_CAPACITY];



int addMnemonic(char* string, int length) {
	Mnemonic mnemonic = { };

	for(int i = 0; i < length; i++)
		mnemonic.chars[i] = string[i];

	int index = mnemonicsSize;

	for(int i = 0; i < mnemonicsSize; i++) {
		if(memcmp(mnemonic.chars, mnemonics[i].chars, 16) == 0) {
			index = i;
			break;
		}
	}

	if(index == mnemonicsSize)
		mnemonics[mnemonicsSize++] = mnemonic;

	return index;
}



static inline void skipSpaces() {
	while(chars[pos] == ' ') pos++;
}



static inline void skipLine() {
	while(chars[pos++] != '\n') { }
}



static inline int atNewline() {
	return chars[pos] == '\r' || chars[pos] == '\n';
}



static int parseOperands() {
	u64 value = parseAscii8();

	for(int i = 0; i < OPERANDS2_COUNT; i++) {
		if(value != compoundOperandsAscii8[i]) continue;

		switch(i) {
			case OPERANDS2_RM_R:  return operandsBit(OPERANDS_R_R)  | operandsBit(OPERANDS_M_R);
			case OPERANDS2_R_RM:  return operandsBit(OPERANDS_R_R)  | operandsBit(OPERANDS_R_M);
			case OPERANDS2_RM_I:  return operandsBit(OPERANDS_R_I)  | operandsBit(OPERANDS_M_I);
			case OPERANDS2_RM_I8: return operandsBit(OPERANDS_R_I8) | operandsBit(OPERANDS_M_I8);
		}
	}

	for(int i = 0; i < OPERANDS_COUNT; i++) {
		if(value != operandsAscii8[i]) continue;
	}

}



static void parse(char* path) {
	init();
	readFile(getLocalFile(path));

	while(pos < size) {
		char c = chars[pos];

		if(c == ';')
			break;

		if(c == '\n' || c == '\r') {
			pos++;
			continue;
		}

		if(c == '#') {
			while(chars[pos++] != '\n') { }
			continue;
		}

		char* opcodeString = &chars[pos];
		int opcodeLength = 0;
		while(opcodeString[opcodeLength] != ' ' && opcodeString[opcodeLength] != '/') opcodeLength++;
		pos += opcodeLength;
		int opcode = parseHexString(opcodeString, opcodeLength);
		int extension = 0;

		if(chars[pos] == '/') {
			pos++;
			extension = chars[pos++] - '0';
			if(extension < 0 || extension > 9)
				error("Invalid extension: %d", extension);
		}

		while(chars[pos] == ' ') pos++;

		char* mnemonicString = &chars[pos];
		int mnemonicLength = 0;
		while(mnemonicString[mnemonicLength] != ' ' && mnemonicString[mnemonicLength] != '\n') mnemonicLength++;
		pos += mnemonicLength;

		int mnemonic = addMnemonic(mnemonicString, mnemonicLength);

		int operands;
		u64 widthsString;

		skipSpaces();
		if(!atNewline()) {
			operands = parseOperands();
			skipSpaces();
			if(!atNewline())
				widthsString = parseAscii8();
		}

		skipLine();
	}

	for(int i = 0; i < mnemonicsSize; i++) {
		//printf("%s\n", mnemonics[i].chars);
		//printf("%s, ", mnemonics[i].chars);
		//if(i % 4 == 3) printf("\n");
	}
}



int main() {
	parse("gen/encodings.txt");
}