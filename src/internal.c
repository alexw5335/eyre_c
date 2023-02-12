#include "internal.h"
#include <stdlib.h>
#include <string.h>

#include <fileapi.h>
#include <handleapi.h>
#include <processenv.h>
#include <processthreadsapi.h>
#include <synchapi.h>



// Uncategorised



// Primitives



int isImm8(s64 input) {
	return input >= I8_MIN && input <= I8_MAX;
}

int isImm16(s64 input) {
	return input >= I16_MIN && input <= I16_MAX;
}

int isImm32(s64 input) {
	return input >= I32_MIN && input <= I32_MAX;
}



// Variables



DllImport dllImports[dllImportCapacity];

int dllImportCount;


Relocation relocations[relocationCapacity];

int relocationCount;



// Memory



static const int persistentSize = 1 << 20; // 1MB

static void* persistentStart;

static void* persistentCurrent;

static void* persistentEnd;



void* palloc(int size) {
	if(size > persistentSize)
		error("Persistent allocation is too large: %s", size);

	persistentCurrent = (void*) ((unsigned long long) persistentCurrent + 7 & -8);

	if(persistentCurrent + size > persistentEnd) {
		persistentStart = malloc(persistentSize);
		persistentCurrent = persistentStart;
		persistentEnd = persistentCurrent + persistentSize;
	}

	void* pointer = persistentCurrent;
	persistentCurrent += size;
	return pointer;
}



void checkCapacity(void** pData, int size, int* pCapacity, int elementSize) {
	int capacity = *pCapacity * elementSize;
	if(*pData == NULL) {
		*pData = malloc(capacity);
	} else if(size >= capacity) {
		*pCapacity = size << 2;
		*pData = realloc(*pData, size << 2);
	}
}



// Logging



void printPointer(void* value) {
	printf("%p\n", value);
}

void printInt(int value) {
	printf("%d\n", value);
}

void printString(char* value) {
	printf("%s\n", value);
}

void println(char* format, ...) {
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	printf("\n");
}

void printNewline() {
	printf("\n");
}

void errorAt(char* format, const char* file, int line, ...) {
	va_list args;
	va_start(args, line);
	fprintf(stdout, "Error at %s:%d: ", file, line);
	vfprintf(stdout, format, args);
	fprintf(stdout, "\n");
	exit(1);
}



// Commands



void runCommand(char* command) {
	STARTUPINFOA startUpInfo = { };
	PROCESS_INFORMATION processInfo = { };

	int result = CreateProcessA(
		NULL,
		command,
		NULL,
		NULL,
		1,
		0,
		NULL,
		NULL,
		&startUpInfo,
		&processInfo
	);

	if(result == 0)
		error("CreateProcess failed for command \"%s\"", command);

	WaitForSingleObject(processInfo.hProcess, 5000);

	CloseHandle(processInfo.hProcess);
}



void runCommandArgs(int num, ...) {
	int capacity = 256;
	char buffer[capacity];
	int pos = 0;
	va_list list;
	va_start(list, num);
	for(int i = 0; i < num; i++) {
		char* arg = va_arg(list, char*);
		int length = strlen(arg);
		if(pos + length + 4 >= capacity)
			error("Command builder buffer overflow");
		buffer[pos++] = '"';
		memcpy(&buffer[pos], arg, length);
		pos += length;
		buffer[pos++] = '"';
		buffer[pos++] = ' ';
	}
	buffer[pos] = 0;
	runCommand(buffer);
}



// Files



static char* fileBuffer         = NULL;
static int   fileBufferCapacity = 8192;
static int   fileBufferSize     = 0;



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

	fileBufferSize = (int) GetFileSize(handle, NULL);

	if(fileBuffer == NULL) {
		fileBufferCapacity = fileBufferSize > fileBufferCapacity ? (fileBufferSize << 2) : fileBufferCapacity;
		fileBuffer = malloc(fileBufferCapacity);
	} else if(fileBufferSize > fileBufferCapacity) {
		fileBufferCapacity = fileBufferSize << 2;
		fileBuffer = malloc(fileBufferCapacity);
	}

	unsigned long numBytesRead;
	int readResult = ReadFile(handle, fileBuffer, fileBufferSize, &numBytesRead, NULL);
	if(readResult == 0)
		error("Error reading file: %s", path);

	// Pad end with zeroes
	*((long long*)(&fileBuffer[fileBufferSize])) = 0;

	if(!CloseHandle(handle))
		error("Failed to close handle to file \"%s\"", path);
}



int getReadFileLength() {
	return fileBufferSize;
}



char* getReadFileData() {
	return fileBuffer;
}



char* getLocalFile(char* fileName) {
	int fileNameLength = (int) strlen(fileName);
	int length = (int) GetCurrentDirectoryA(0, NULL);
	char* file = malloc(length + fileNameLength); // Must be freed by caller
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



void writeFile(char* path, int dataSize, void* data) {
	HANDLE handle = CreateFileA(
		path,
		GENERIC_WRITE,
		FILE_SHARE_WRITE,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if(handle == INVALID_HANDLE_VALUE)
		error("Failed to create writeable file: \"%s\"", path);

	unsigned long written;
	if(!WriteFile(handle, data, dataSize, &written, NULL))
		error("Failed to write %d bytes from %p to file \"%s\"", dataSize, data, path);

	if(!CloseHandle(handle))
		error("Failed to close handle to file \"%s\"", path);
}