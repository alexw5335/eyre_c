#ifndef INCLUDE_BUFFER
#define INCLUDE_BUFFER

#include "intrin.h"



#ifndef bufferCapacity
#define bufferCapacity 66536
#endif



static char buffer[bufferCapacity + 8];
static void* bufferPos = buffer;
static void* bufferEnd = buffer + bufferCapacity;



static inline int getBufferLength() {
	return (int) (bufferPos - (void*) buffer);
}

static inline void checkBufferExtra(int length) {
	if(bufferPos + length >= bufferEnd)
		error("Capacity reached");
}

static inline void checkBuffer() {
	if(bufferPos >= bufferEnd)
		error("Capacity reached");
}

static void write8(char value) {
	*(char*) bufferPos = value;
	bufferPos++;
	checkBuffer();
}

static void write16(short value) {
	*(short*) bufferPos = value;
	bufferPos += 2;
	checkBuffer();
}

static void write32(int value) {
	*(int*) bufferPos = value;
	bufferPos += 4;
	checkBuffer();
}

static void write64(long long value) {
	*(long long*) bufferPos = value;
	bufferPos += 8;
	checkBuffer();
}

static void* bufferSeek(int pos) {
	void* prev = bufferPos;
	bufferPos = buffer + pos;
	checkBuffer();
	return prev;
}

static void bufferSetPos(void* pos) {
	bufferPos = pos;
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
	memcpy(bufferPos, data, length);
	bufferPos += length;
}

static void writeVarLengthInt(int value) {
	*(int*) bufferPos = value;
	bufferPos += (7 + _bit_scan_reverse(value | 1) & -8) >> 3;
	checkBuffer();
}

static int writeWidth(int width, s64 value) {
	switch(width) {
		case 0: {
			return TRUE;
		}
		case 1: {
			if(!isImm8(width)) return FALSE;
			write8(value);
			return TRUE;
		}
		case 2: {
			if(!isImm16(width)) return FALSE;
			write16(value);
			return TRUE;
		}
		case 3: {
			if(!isImm32(width)) return FALSE;
			write32(value);
			return TRUE;
		}
		case 4: {
			write64(value);
			return TRUE;
		}
		default: return FALSE;
	}
}



#endif