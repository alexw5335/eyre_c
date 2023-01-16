#include <fileapi.h>
#include <handleapi.h>
#include <stdarg.h>
#include "eyre.h"



// LOGGING



void printPointer(void* value) {
	printf("%p\n", value);
}



void printInt(int value) {
	printf("%d\n", value);
}



void println(char* format, ...) {
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	printf("\n");
}

void eyreLogError_(char* format, const char* file, int line, ...) {
	va_list args;
	va_start(args, line);
	fprintf(stderr, "ERROR %s:%d: ", file, line);
	vfprintf(stderr, format, args);
	fprintf(stderr, "\n");
	exit(1);
}


void eyreLogWarning_(char* format, const char* file, int line, ...) {
	va_list args;
	va_start(args, line);
	printf("WARN  %s:%d: ", file, line);
	vprintf(format, args);
	printf("\n");
}

void eyreLogDebug_(char* format, const char* file, int line, ...) {
	va_list args;
	va_start(args, line);
	printf("DEBUG %s:%d: ", file, line);
	vprintf(format, args);
	printf("\n");
}

void eyreLogInfo_(char* format, const char* file, int line, ...) {
	va_list args;
	va_start(args, line);
	printf("INFO  %s:%d: ", file, line);
	vprintf(format, args);
	printf("\n");
}

void eyreLogTrace_(char* format, const char* file, int line, ...) {
	va_list args;
	va_start(args, line);
	printf("TRACE %s:%d: ", file, line);
	vprintf(format, args);
	printf("\n");
}



void createSrcFile(SrcFile* srcFile, char* path) {
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
		eyreLogError("Error opening file: %s", path);

	int size = (int) GetFileSize(handle, NULL);
	char* data = eyreAlloc(size + 4);
	data[size] = 0;
	data[size + 1] = 0;
	data[size + 2] = 0;
	data[size + 3] = 0;

	srcFile->size = size;
	srcFile->data = data;

	unsigned long numBytesRead;
	int readResult = ReadFile(handle, srcFile->data, srcFile->size, &numBytesRead, NULL);
	if(readResult == 0) eyreLogError("Error reading file: %s", path);

	CloseHandle(handle);
}



// Allocation



void* eyreAlloc(int size) {
	return malloc(size);
}



void* eyreRealloc(void* pointer, int size) {
	return realloc(pointer, size);
}



void eyreFree(void* pointer) {
	free(pointer);
}



static const int persistentSize = 1 << 16;

static void* persistentCurrent;

static void* persistentEnd;



void* eyreAllocPersistent(int size) {
	if(persistentCurrent + size > persistentEnd) {
		persistentCurrent = malloc(persistentSize);
		persistentEnd = persistentCurrent + persistentSize;;
	}

	void* pointer = persistentCurrent;
	persistentCurrent += size;
	return pointer;
}



void listEnsureCapacity(List* list, int elementSize) {
	if(list->data == NULL) {
		if(list->capacity <= 0)
			eyreLogError("List initial capacity is zero");
		list->data = malloc(list->capacity * elementSize);
		if(list->data == NULL)
			eyreLogError("Failed to allocate memory for a list");
	} else if(list->size >= list->capacity) {
		list->capacity = list->size << 2;
		list->data = realloc(list->data, list->capacity * elementSize);
		if(list->data == NULL)
			eyreLogError("Failed to reallocate memory for a list");
	}
}



