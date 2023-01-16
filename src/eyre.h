#pragma once

#include <stdio.h>



// Types



typedef unsigned char      u8;
typedef signed char        s8;
typedef unsigned short     u16;
typedef signed short       s16;
typedef unsigned int       u32;
typedef signed int         s32;
typedef unsigned long long u64;
typedef signed long long   s64;




// Structs



typedef struct SrcFile {
	char* data;
	int size;
} SrcFile;



typedef struct {
	void* data;
	int size;
	int capacity;
} List;



typedef struct {
	int id;
	int hash;
	int length;
	int flags;
	char* string;
} Intern;



typedef struct {
	int intern;
	int next;
} Node;




// Functions



void createSrcFile(SrcFile* srcFile, char* path);

void eyreLex(SrcFile* srcFile);

int eyreIntern(char* string, int length);

void listEnsureCapacity(List* list, int elementSize);



// Allocation



void* eyreAlloc(int size);

void* eyreRealloc(void* pointer, int size);

void eyreFree(void* pointer);

void* eyreAllocPersistent(int size);

void checkListCapacity(List* list, int elementSize);



// PRINTING



void printPointer(void* value);

void printInt(int value);

void println(char* format, ...);



// LOGGING
// EYRE_LOG_INFO, EYRE_LOG_DEBUG, EYRE_LOG_ALL



// Always results in termination, logged to stderr
void eyreLogError_(char* format, const char* file, int line, ...);

// Always logged
void eyreLogWarning_(char* format, const char* file, int line, ...);

// Important debug messages
void eyreLogDebug_(char* format, const char* file, int line, ...);

// Unimportant debug messages
void eyreLogInfo_(char* format, const char* file, int line, ...);

// Only for targeted debugging
void eyreLogTrace_(char* format, const char* file, int line, ...);

#define eyreLogError(format, ...) eyreLogError_(format, __FILE__, __LINE__, ##__VA_ARGS__)

#define eyreLogWarning(format, ...) eyreLogWarning_(format, __FILE__, __LINE__, ##__VA_ARGS__)

#if defined(EYRE_LOG_DEBUG) | defined(EYRE_LOG_ALL)
#define eyreLogDebug(format, ...) eyreLogDebug_(format, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define eyreLogDebug(format, ...)
#endif

#if defined(EYRE_LOG_INFO) | defined(EYRE_LOG_ALL)
#define eyreLogInfo(format, ...) eyreLogInfo_(format, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define eyreLogInfo(format, ...)
#endif

#if defined(EYRE_LOG_TRACE) | defined(EYRE_LOG_ALL)
#define eyreLogTrace(format, ...) eyreLogTrace_(format, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define eyreLogTrace(format, ...)
#endif