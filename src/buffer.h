#ifndef INCLUDE_BUFFER
#define INCLUDE_BUFFER

#include "intrin.h"
#include "enums.h"



#ifndef bufferCapacity
#define bufferCapacity 100000
#endif



// Padding bytes allow for length checks at the end of functions
static char buffer[bufferCapacity + 8];
static int bufferPos = 0;



static inline void checkBufferExtra(int length) {
	if(bufferPos + length >= bufferCapacity)
		error("Capacity reached");
}

static inline void checkBuffer() {
	if(bufferPos >= bufferCapacity)
		error("Capacity reached");
}



static void write8At(int pos, int value) {
	buffer[pos] = value;
}

static void write16At(int pos, int value) {
	*(short*) &buffer[pos] = value;
}

static void write32At(int pos, int value) {
	*(int*) &buffer[pos] = value;
}

static void write64At(int pos, int value) {
	*(long long*) &buffer[pos] = value;
}



static void write8(char value) {
	buffer[bufferPos++] = value;
	checkBuffer();
}

static void write16(short value) {
	*(short*) &buffer[bufferPos] = value;
	bufferPos += 2;
	checkBuffer();
}

static void write32(int value) {
	*(int*) &buffer[bufferPos] = value;
	bufferPos += 4;
	checkBuffer();
}

static void write64(long long value) {
	*(long long*) &buffer[bufferPos] = value;
	bufferPos += 8;
	checkBuffer();
}



static int bufferSeek(int pos) {
	int prev = bufferPos;
	bufferPos = pos;
	checkBuffer();
	return prev;
}

static void zeroTo(int pos) {
	int length = pos - bufferPos;
	checkBufferExtra(length);
	memset(&buffer[bufferPos], 0, length);
	bufferPos = pos;
}

static void writeZero(int length) {
	checkBufferExtra(length);
	memset(&buffer[bufferPos], 0, length);
	bufferPos += length;
}



static void writeAsciiNt(char* ascii) {
	int length = strlen(ascii);
	checkBufferExtra(length + 1);
	for(int i = 0; i < length; i++)
		buffer[bufferPos++] = ascii[i];
	buffer[bufferPos++] = 0;
}

static void writeAscii(char* ascii) {
	int length = strlen(ascii);
	checkBufferExtra(length);
	for(int i = 0; i < length; i++)
		buffer[bufferPos++] = ascii[i];
}

static void writeAscii64(char* ascii) {
	long long value = 0;
	int length = strlen(ascii);
	for(int i = 0; i < length; i++)
		value = (value << 8) | ascii[length - i - 1];
	write64(value);
}

static void bufferAdvance(int count) {
	bufferPos += count;
	checkBuffer();
}

static void writeBytes(void* data, int length) {
	checkBufferExtra(length);
	memcpy(&buffer[bufferPos], data, length);
	bufferPos += length;
}

static void writeVarLengthInt(int value) {
	*(int*) &buffer[bufferPos] = value;
	bufferPos += (8 + (_bit_scan_reverse(value | 1) & -8)) >> 3;
	checkBuffer();
}

static int writeWidth(int width, s64 value) {
	switch(width) {
		case WIDTH_BYTE: {
			if(!isImm8(width)) return FALSE;
			write8(value);
			return TRUE;
		}
		case WIDTH_WORD: {
			if(!isImm16(width)) return FALSE;
			write16(value);
			return TRUE;
		}
		case WIDTH_DWORD: {
			if(!isImm32(width)) return FALSE;
			write32(value);
			return TRUE;
		}
		case WIDTH_QWORD: {
			write64(value);
			return TRUE;
		}
		default: return FALSE;
	}
}



#endif