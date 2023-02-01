#include <fileapi.h>
#include <handleapi.h>
#include "eyre.h"
#include <processenv.h>
#include "log.h"
#include <processthreadsapi.h>
#include <synchapi.h>



// Util



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



// File



void eyreCreateSrcFile(SrcFile* srcFile, char* path) {
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
		eyreError("Error opening file: %s", path);

	int size = (int) GetFileSize(handle, NULL);
	char* data = eyreAlloc(size + 4);
	data[size] = 0;
	data[size + 1] = 0;
	data[size + 2] = 0;
	data[size + 3] = 0;

	srcFile->path = path;
	srcFile->size = size;
	srcFile->chars = data;

	unsigned long numBytesRead;
	int readResult = ReadFile(handle, srcFile->chars, srcFile->size, &numBytesRead, NULL);
	if(readResult == 0) eyreError("Error reading file: %s", path);

	CloseHandle(handle);
}



char* eyreGetLocalFile(char* fileName) {
	int fileNameLength = (int) strlen(fileName);
	int length = (int) GetCurrentDirectoryA(0, NULL);
	char* file = eyreAlloc(length + fileNameLength);
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



void eyreRunCommand(char* command) {
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
		eyreError("CreateProcess failed for command \"%s\"", command);

	WaitForSingleObject(processInfo.hProcess, 5000);

	CloseHandle(processInfo.hProcess);
}



void eyreRunCommandArgs(int num, ...) {
	int capacity = 256;
	char buffer[capacity];
	int pos = 0;
	va_list list;
	va_start(list, num);
	for(int i = 0; i < num; i++) {
		char* arg = va_arg(list, char*);
		int length = strlen(arg);
		if(pos + length + 4 >= capacity)
			eyreError("Command builder buffer overflow");
		buffer[pos++] = '"';
		memcpy(&buffer[pos], arg, length);
		pos += length;
		buffer[pos++] = '"';
		buffer[pos++] = ' ';
	}
	buffer[pos] = 0;
	eyreRunCommand(buffer);
}



void eyreWriteFile(char* path, int dataSize, void* data) {
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
		eyreError("Failed to create writeable file: \"%s\"", path);

	unsigned long written;
	if(!WriteFile(handle, data, dataSize, &written, NULL))
		eyreError("Failed to write %d bytes from %p to file \"%s\"", dataSize, data, path);

	if(!CloseHandle(handle))
		eyreError("Failed to close handle to file \"%s\"", path);
}



// Allocation



inline void checkCapacity(void** pData, int size, int* pCapacity, int elementSize) {
	int capacity = *pCapacity * elementSize;
	if(*pData == NULL) {
		*pData = malloc(capacity);
	} else if(size >= capacity) {
		*pCapacity = size << 2;
		*pData = realloc(*pData, size << 2);
	}
}



void* eyreAlloc(int size) {
	return malloc(size);
}



void* eyreRealloc(void* pointer, int size) {
	return realloc(pointer, size);
}



void eyreFree(void* pointer) {
	free(pointer);
}



static const int persistentSize = 1 << 20; // 1MB

static void* persistentStart;

static void* persistentCurrent;

static void* persistentEnd;



static void newPersistentBlock() {
	persistentStart = eyreAlloc(persistentSize);
	persistentCurrent = persistentStart;
	persistentEnd = persistentCurrent + persistentSize;
}



void eyreAllocPersistentContiguous(void** start, int size) {
	if(persistentCurrent + size > persistentEnd) {
		newPersistentBlock();
		*start = persistentCurrent;
	}

	persistentCurrent += size;
}



void* eyreAllocPersistent(int size) {
	persistentCurrent = (void*) ((u64) persistentCurrent + 7 & -8);

	if(persistentCurrent + size > persistentEnd)
		newPersistentBlock();

	void* pointer = persistentCurrent;
	persistentCurrent += size;
	return pointer;
}



void eyreCheckListCapacity(List* list, int elementSize) {
	if(list->data == NULL) {
		if(list->capacity <= 0)
			eyreError("List initial capacity is zero");
		list->data = eyreAlloc(list->capacity * elementSize);
		if(list->data == NULL)
			eyreError("Failed to allocate memory for a list");
	} else if(list->size >= list->capacity) {
		list->capacity = list->size << 2;
		list->data = eyreRealloc(list->data, list->capacity * elementSize);
		if(list->data == NULL)
			eyreError("Failed to reallocate memory for a list");
	}
}



void* eyreBufferAlloc(Buffer* buffer, int size) {
	if(size > 8)
		size = (size + 7) & -8;

	if(buffer->current + size >= buffer->end) {
		eyreCheckListCapacity(&buffer->banks, sizeof(void*));
		void** banks = (void**) buffer->banks.data;
		banks[buffer->banks.size++] = buffer->start;
		buffer->start = malloc(buffer->bankCapacity);
		if(buffer->start == NULL)
			eyreError("Failed to allocate buffer memory");
		buffer->current = buffer->start;
		buffer->end = buffer->start + buffer->bankCapacity;
	}

	void* pointer = buffer->current;
	buffer->current += size;
	return pointer;
}