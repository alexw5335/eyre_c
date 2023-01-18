#include <fileapi.h>
#include <handleapi.h>
#include "eyre.h"
#include <processenv.h>
#include "log.h"



void eyreCreateSrcFileFromFile(SrcFile* srcFile, char* path) {
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

	srcFile->path = path;
	srcFile->size = size;
	srcFile->chars = data;

	unsigned long numBytesRead;
	int readResult = ReadFile(handle, srcFile->chars, srcFile->size, &numBytesRead, NULL);
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



/*static const int persistentSize = 1 << 16;

static void* persistentCurrent;

static void* persistentEnd;



void* eyreAllocPersistent(int size) {
	if(persistentCurrent + size > persistentEnd) {
		persistentCurrent = eyreAlloc(persistentSize);
		persistentEnd = persistentCurrent + persistentSize;
	}

	void* pointer = persistentCurrent;
	persistentCurrent += size;
	return pointer;
}*/



void eyreCheckListCapacity(List* list, int elementSize) {
	if(list->data == NULL) {
		if(list->capacity <= 0)
			eyreLogError("List initial capacity is zero");
		list->data = eyreAlloc(list->capacity * elementSize);
		if(list->data == NULL)
			eyreLogError("Failed to allocate memory for a list");
	} else if(list->size >= list->capacity) {
		list->capacity = list->size << 2;
		list->data = eyreRealloc(list->data, list->capacity * elementSize);
		if(list->data == NULL)
			eyreLogError("Failed to reallocate memory for a list");
	}
}



char* eyreGetFileInCurrentDirectory(char* fileName) {
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