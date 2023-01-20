#include <fileapi.h>
#include <handleapi.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <processenv.h>



void error(char* format, ...) {
	va_list list;
	va_start(list, format);
	vfprintf(stderr, format, list);
	fprintf(stderr, "\n");
	exit(1);
}



char* chars;

int size;

int pos;



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



void parse(char* path) {
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

		printf("%x %.*s\n", opcode, mnemonicLength, mnemonicString);
		while(chars[pos++] != '\n') { }
	}
}



int main() {
	parse("gen/encodings.txt");
}