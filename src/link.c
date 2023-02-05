#include "assemble.h"
#include "log.h"
#include <string.h>
#include "eyre.h"



#define bufferCapacity 66536
static char buffer[bufferCapacity + 8];
static void* bufferPos = buffer;
static void* bufferEnd = buffer + bufferCapacity;

#define sectionAlignment 0x1000
#define fileAlignment 0x200

static inline int roundToSectionAlignment(int value) {
	return (value + sectionAlignment - 1) & -sectionAlignment;
}

static inline int roundToFileAlignment(int value) {
	return (value + fileAlignment - 1) & -fileAlignment;
}



static inline void checkBufferExtra(int length) {
	if(bufferPos + length >= bufferEnd)
		eyreError("Capacity reached");
}

static inline void checkBuffer() {
	if(bufferPos >= bufferEnd)
		eyreError("Capacity reached");
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

static void seek(int pos) {
	bufferPos = buffer + pos;
	checkBuffer();
}

static void writeAscii64(char* ascii) {
	long long value = 0;
	for(int i = 0; i < strlen(ascii); i++)
		value = (value << 8) | ascii[i];
	write64(value);
}

static void advance(int count) {
	bufferPos += count;
	checkBuffer();
}

static void bytes(void* data, int length) {
	checkBufferExtra(length);
	memcpy(bufferPos, data, length);
	bufferPos += length;
}


static void writeHeaders() {
	write16(0x5A4D);
	seek(0x3C);
	write32(0x40);

	write32(0x4550);     // signature
	write16(0x8664);     // machine
	write16(1);          // numSections    (fill in later)
	write32(0);          // timeDateStamp
	write32(0);          // pSymbolTable
	write32(0);          // numSymbols
	write16(0xF0);       // optionalHeaderSize
	write16(0x0022);     // characteristics, DYNAMIC_BASE | LARGE_ADDRESS_AWARE | EXECUTABLE

	write16(0x20B);      // magic
	write16(0);          // linkerVersion
	write32(0);          // sizeOfCode
	write32(0);          // sizeOfInitialisedData
	write32(0);          // sizeOfUninitialisedData
	write32(0x1000);     // pEntryPoint    (fill in later);
	write32(0);          // baseOfCode
	write64(0x400000);   // imageBase
	write32(0x1000);     // sectionAlignment
	write32(0x200);      // fileAlignment
	write16(6);          // majorOSVersion
	write16(0);          // minorOSVersion
	write32(0);          // imageVersion
	write16(6);          // majorSubsystemVersion
	write16(0);          // minorSubsystemVersion
	write32(0);          // win32VersionValue
	write32(0);          // sizeOfImage    (fill in later);
	write32(0x200);      // sizeOfHeaders
	write32(0);          // checksum
	write16(3);          // subsystem
	write16(0x140);      // dllCharacteristics
	write64(0x100000);   // stackReserve
	write64(0x1000);     // stackCommit
	write64(0x100000);   // heapReserve
	write64(0x1000);     // heapCommit
	write32(0);          // loaderFlags
	write32(16);         // numDataDirectories
	advance(16 * 8);     // dataDirectories (fill in later)
	seek(0x200);         // section headers (fill in later)
}



static void writeSections() {
	int rawDataPos = fileAlignment;
	int rawDataSize = roundToFileAlignment(eyreGetAssemblerBufferSize());
	int virtualSize = eyreGetAssemblerBufferSize();
	int virtualAddress = sectionAlignment;

	seek(328); // Section headers start
	writeAscii64(".text");
	write32(virtualSize);
	write32(virtualAddress);
	write32(rawDataSize);
	write32(rawDataPos);
	advance(12);
	write32(0x60000020 | 0x40000040);

	seek(rawDataPos);
	bytes(eyreGetAssemblerBuffer(), rawDataSize);
}



void eyreLink() {
	writeHeaders();
	writeSections();
}



void* eyreGetLinkBuffer() {
	return buffer;
}



int eyreGetLinkBufferSize() {
	return (int) (bufferPos - (void*) buffer);
}