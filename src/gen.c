#include <fileapi.h>
#include <handleapi.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <processenv.h>
#include "defs.h"
#include "gen.h"



static void error(char* format, ...) {
	va_list list;
	va_start(list, format);
	vfprintf(stderr, format, list);
	fprintf(stderr, "\n");
	exit(1);
}



// Variables



static char* chars;

static int size;

static int pos;

#define ENCODINGS_CAPACITY 1024
static int encodingCount;
static EyreGenEncoding encodings[ENCODINGS_CAPACITY];

#define GROUPS_CAPACITY 512
static int groupCount;
static EyreGenGroup groups[GROUPS_CAPACITY];



// File



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



// Parsing utils



static void skipSpaces() {
	while(chars[pos] == ' ') pos++;
}



static void skipLine() {
	while(chars[pos++] != '\n') { }
}



static int atNewline() {
	return chars[pos] == '\r' || chars[pos] == '\n';
}



static int isWhitespace(char c) {
	return c == ' ' || c == '\r' || c == '\n' || c == '\t';
}



// Integer parsing



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



// String parsing



static int stringLength() {
	char* string = &chars[pos];
	int length = 0;
	while(1) {
		char c = string[length];
		if(isWhitespace(c)) break;
		length++;
	}
	return length;
}



static char* parseString(int length) {
	char* string = &chars[pos];
	pos += length;
	return string;
}



// Struct creation



static int addEncoding(EyreGenGroup* group, int opcode, int extension, EyreOperands operands, int widths) {
	if(encodingCount >= ENCODINGS_CAPACITY)
		error("Too many encodings");

	EyreGenEncoding* encoding  = &encodings[encodingCount++];
	encoding->mnemonic  = group->mnemonic;
	encoding->opcode    = opcode;
	encoding->extension = extension;
	encoding->operands  = operands;
	encoding->widths    = widths;

	if(group->encodingCount >= 32)
		error("Too many encodings for mnemonic %s", group->mnemonic);

	group->encodings[group->encodingCount++] = encoding;
	group->operandsBits |= (1 << encoding->operands);
	group->specifierBits |= (1 << eyreOperandsSpecifiers[encoding->operands]);

	return encodingCount - 1;
}



static EyreGenGroup* addGroup(char* string, int length) {
	for(int i = 0; i < groupCount; i++)
		if(length == strlen(groups[i].mnemonic) && memcmp(string, groups[i].mnemonic, length) == 0)
			return &groups[i];

	if(groupCount >= GROUPS_CAPACITY)
		error("Too many groups");

	EyreGenGroup* group = &groups[groupCount++];
	memcpy(group->mnemonic, string, length);
	return group;
}



// Parsing



static int parseOperands(char* string, int length) {
	for(int i = 0; i < OPERANDS_COUNT; i++)
		if(length == strlen(eyreOperandsNames[i]) && memcmp(string, eyreOperandsNames[i], length) == 0)
			return i;

	return -1;
}



static int parseCompound(char* string, int length) {
	for(int i = 0; i < COMPOUND_COUNT; i++)
		if(length == strlen(eyreCompoundOperandsNames[i]) && memcmp(string, eyreCompoundOperandsNames[i], length) == 0)
			return i;

	return -1;
}



static int parseOpcode() {
	char* opcodeString = &chars[pos];
	int opcodeLength = 0;
	while(opcodeString[opcodeLength] != ' ' && opcodeString[opcodeLength] != '/') opcodeLength++;
	pos += opcodeLength;
	return parseHexString(opcodeString, opcodeLength);
}



static int parseExtension() {
	if(chars[pos] != '/') return 0;
	pos++;
	int extension = chars[pos++] - '0';
	if(extension < 0 || extension > 9)
		error("Invalid extension: %d", extension);
	return extension;
}



static EyreGenGroup* parseGroup() {
	int mnemonicLength = stringLength();
	char* mnemonicString = parseString(mnemonicLength);
	return addGroup(mnemonicString, mnemonicLength);
}



static int parseWidths() {
	if(atNewline()) return 0;
	if(chars[pos] != '0' && chars[pos] != '1') return 0;
	int widths = 0;
	for(int i = 0; i < 4; i++) {
		char c = chars[pos++];
		if(c == '1')
			widths |= (1 << i);
		else if(c != '0')
			error("Invalid widths char: %d", c);
	}
	return widths;
}



// Parsing



void eyreParseEncodings(char* path) {
	readFile(getLocalFile(path));

	while(pos < size) {
		char c = chars[pos];

		if(c == ';') break;

		if(isWhitespace(c)) {
			pos++;
			continue;
		}

		if(c == '#') {
			skipLine();
			continue;
		}

		int opcode = parseOpcode();
		int extension = parseExtension();
		skipSpaces();

		EyreGenGroup* group = parseGroup();
		skipSpaces();

		int operandsLength = stringLength();
		char* operandsString = parseString(operandsLength);
		int operands = parseOperands(operandsString, operandsLength);
		int compound = parseCompound(operandsString, operandsLength);
		skipSpaces();

		int widths = parseWidths();
		skipLine();

		if(compound >= 0) {
			EyreOperands* compoundOperands = eyreCompoundOperandsMap[compound];
			for(int i = 0; i < 5; i++) {
				if(compoundOperands[i] == 0) break;
				addEncoding(group, opcode, extension, compoundOperands[i], widths);
			}
		} else if(operands >= 0) {
			addEncoding(group, opcode, extension, operands, widths);
		} else if(operandsLength == 0) {
			addEncoding(group, opcode, extension, OPERANDS_NONE, widths);
		} else {
			error("Invalid operands: %.*s", operandsLength, operandsString);
		}
	}
}



void eyreGenGroups() {
	printf("#include \"eyre_defs.h\"\n\n");

	for(int i = 0; i < groupCount; i++) {
		EyreGenGroup g = groups[i];

		printf("static EyreEncoding EYRE_ENCODINGS_%s[] = {\n", g.mnemonic);
		for(int j = 0; j < g.encodingCount; j++) {
			EyreGenEncoding* e = g.encodings[j];
			printf("\t{ %d, %d, %d, %d },\n", e->opcode, e->extension, 0, e->widths);
		}
		printf("};\n\n");
	}

	printf("static EyreGroup eyreEncodings[] = {\n");
	for(int i = 0; i < groupCount; i++) {
		EyreGenGroup g = groups[i];
		printf("\t{ %d, %d, EYRE_ENCODINGS_%s },\n", g.operandsBits, g.specifierBits, g.mnemonic);
	}
	printf("};\n");

	printf("\nEyreGroup eyreGetEncodings(int mnemonic) { return eyreEncodings[mnemonic]; }\n\n\n");
}



void eyreGenMnemonics() {
	printf("typedef enum EyreMnemonic {\n");
	for(int i = 0; i < groupCount; i++) {
		if(i % 4 == 0) printf("\t");
		printf("MNEMONIC_%s, ", groups[i].mnemonic);
		if(i % 4 == 3) printf("\n");
	}
	if(groupCount % 4 != 3) printf("\n");
	printf("\tMNEMONIC_COUNT\n");
	printf("} EyreMnemonic;\n");

	printf("\nstatic char* eyreMnemonicNames[MNEMONIC_COUNT] = {");
	for(int i = 0; i < groupCount; i++) {
		char lowercase[16];
		for(int j = 0; j < 16; j++) {
			char c = groups[i].mnemonic[j];
			if(c >= 'A' && c <= 'Z') lowercase[j] = c - 'A' + 'a'; else lowercase[j] = c;
		}
		if(i % 4 == 0) printf("\t");
		printf("\"%s\", ", lowercase);
		if(i % 4 == 3) printf("\n");
	}
	if(groupCount % 4 != 3) printf("\n");
	printf("};\n");
}



static void printEncodings() {
	for(int i = 0; i < encodingCount; i++) {
		EyreGenEncoding encoding = encodings[i];
		printf("%x/%d %s", encoding.opcode, encoding.extension, encoding.mnemonic);
		if(encoding.operands >= 0)
			printf(" %s", eyreOperandsNames[encoding.operands]);
		if(encoding.widths > 0)
			printf(" %d", encoding.widths);
		printf("\n");
	}
}