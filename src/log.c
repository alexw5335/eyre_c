#include "log.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>



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



void eyreLogError_(char* format, const char* file, int line, ...) {
	va_list args;
	va_start(args, line);
	fprintf(stderr, "Error at %s:%d: ", file, line);
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