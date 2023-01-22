#include <fileapi.h>
#include <handleapi.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <processenv.h>
#include "defs.h"



void error(char* format, ...) {
	va_list list;
	va_start(list, format);
	vfprintf(stderr, format, list);
	fprintf(stderr, "\n");
	exit(1);
}



// Variables



char* chars;

int size;

int pos;

#define MNEMONICS_CAPACITY 512
int mnemonicCount;
Mnemonic mnemonics[MNEMONICS_CAPACITY];

#define ENCODINGS_CAPACITY 1024
int encodingCount;
Encoding encodings[ENCODINGS_CAPACITY];

#define GROUPS_CAPACITY 512
int groupCount;
EncodingGroup groups[GROUPS_CAPACITY];



// File



void readFile(char* path) {
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



char* getLocalFile(char* fileName) {
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



void skipSpaces() {
	while(chars[pos] == ' ') pos++;
}



void skipLine() {
	while(chars[pos++] != '\n') { }
}



int atNewline() {
	return chars[pos] == '\r' || chars[pos] == '\n';
}



int isWhitespace(char c) {
	return c == ' ' || c == '\r' || c == '\n' || c == '\t';
}



// Integer parsing



int parseHex(unsigned char c) {
	if(c >= '0' && c <= '9')
		return c - '0';
	if(c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if(c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	error("Invalid hex: %d", c);
	return 0;
}



int parseHexString(char* string, int length) {
	int value = 0;
	for(int i = 0; i < length; i++)
		value = (value << 4) | parseHex(string[i]);
	return value;
}



// String parsing



int stringLength() {
	char* string = &chars[pos];
	int length = 0;
	while(1) {
		char c = string[length];
		if(isWhitespace(c)) break;
		length++;
	}
	return length;
}



char* parseString(int length) {
	char* string = &chars[pos];
	pos += length;
	return string;
}



// Struct creation



int addEncoding(int mnemonic, int opcode, int extension, Operands operands, int widths) {
	if(encodingCount >= ENCODINGS_CAPACITY)
		error("Too many encodings");

	Encoding* encoding  = &encodings[encodingCount++];
	encoding->mnemonic  = mnemonic;
	encoding->opcode    = opcode;
	encoding->extension = extension;
	encoding->operands  = operands;
	encoding->widths    = widths;

	return encodingCount - 1;
}



int addMnemonic(char* string, int length) {
	for(int i = 0; i < mnemonicCount; i++)
		if(length == strlen(mnemonics[i].chars) && memcmp(string, mnemonics[i].chars, length) == 0)
			return i;

	memcpy(mnemonics[mnemonicCount++].chars, string, length);

	return mnemonicCount - 1;
}



// Parsing



int parseOperands(char* string, int length) {
	for(int i = 0; i < OPERANDS_COUNT; i++)
		if(length == strlen(operandsNames[i]) && memcmp(string, operandsNames[i], length) == 0)
			return i;

	return -1;
}



int parseCompound(char* string, int length) {
	for(int i = 0; i < COMPOUND_COUNT; i++)
		if(length == strlen(compoundNames[i]) && memcmp(string, compoundNames[i], length) == 0)
			return i;

	return -1;
}



int parseOpcode() {
	char* opcodeString = &chars[pos];
	int opcodeLength = 0;
	while(opcodeString[opcodeLength] != ' ' && opcodeString[opcodeLength] != '/') opcodeLength++;
	pos += opcodeLength;
	return parseHexString(opcodeString, opcodeLength);
}



int parseExtension() {
	if(chars[pos] != '/') return 0;
	pos++;
	int extension = chars[pos++] - '0';
	if(extension < 0 || extension > 9)
		error("Invalid extension: %d", extension);
	return extension;
}



int parseMnemonic() {
	int mnemonicLength = stringLength();
	char* mnemonicString = parseString(mnemonicLength);
	return addMnemonic(mnemonicString, mnemonicLength);
}



int parseWidths() {
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



void parse(char* path) {
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

		int mnemonic = parseMnemonic();
		skipSpaces();

		int operandsLength = stringLength();
		char* operandsString = parseString(operandsLength);
		int operands = parseOperands(operandsString, operandsLength);
		int compound = parseCompound(operandsString, operandsLength);
		skipSpaces();

		int widths = parseWidths();
		skipLine();

		if(compound >= 0) {
			Operands* compoundOperands = compoundMap[compound];
			for(int i = 0; i < 5; i++) {
				if(compoundOperands[i] == 0) break;
				addEncoding(mnemonic, opcode, extension, compoundOperands[i], widths);
			}
		} else if(operands >= 0) {
			addEncoding(mnemonic, opcode, extension, operands, widths);
		} else if(operandsLength == 0) {
			addEncoding(mnemonic, opcode, extension, OPERANDS_NONE, widths);
		} else {
			error("Invalid operands: %.*s", operandsLength, operandsString);
		}
	}
}



void printEncodings(){
	for(int i = 0; i < encodingCount; i++) {
		Encoding encoding = encodings[i];
		char* mnemonic = mnemonics[encoding.mnemonic].chars;
		printf("%x/%d %s", encoding.opcode, encoding.extension, mnemonic);
		if(encoding.operands >= 0)
			printf(" %s", operandsNames[encoding.operands]);
		if(encoding.widths > 0)
			printf(" %d", encoding.widths);
		printf("\n");
	}
}



void printMnemonics() {
	printf("typedef enum EyreMnemonic {\n");
	for(int i = 0; i < mnemonicCount; i++) {
		if(i % 4 == 0) printf("\t");
		printf("MNEMONIC_%s, ", mnemonics[i].chars);
		if(i % 4 == 3) printf("\n");
	}
	if(mnemonicCount % 4 != 3) printf("\n");
	printf("\tMNEMONIC_COUNT\n");
	printf("} EyreMnemonic;\n");

	printf("\nstatic char* eyreMnemonicNames[MNEMONIC_COUNT] = {");
	for(int i = 0; i < mnemonicCount; i++) {
		char lowercase[16];
		for(int j = 0; j < 16; j++) {
			char c = mnemonics[i].chars[j];
			if(c >= 'A' && c <= 'Z') lowercase[j] = c - 'A' + 'a'; else lowercase[j] = c;
		}
		if(i % 4 == 0) printf("\t");
		printf("\"%s\", ", lowercase);
		if(i % 4 == 3) printf("\n");
	}
	if(mnemonicCount % 4 != 3) printf("\n");
	printf("};\n");
}



int main() {
	parse("gen/encodings.txt");
	printEncodings();
}
