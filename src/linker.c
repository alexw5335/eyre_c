#include <string.h>
#include "internal.h"
#include "buffer.h"




#define sectionAlignment 0x1000
#define fileAlignment 0x200

static inline int roundToSectionAlignment(int value) {
	return (value + sectionAlignment - 1) & -sectionAlignment;
}

static inline int roundToFileAlignment(int value) {
	return (value + fileAlignment - 1) & -fileAlignment;
}



static void writeHeaders() {
	write16(0x5A4D);
	bufferSeek(0x3C);
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
	bufferAdvance(16 * 8);     // dataDirectories (fill in later)
	bufferSeek(0x200);         // section headers (fill in later)
}



static void writeSections() {
	int rawDataPos = fileAlignment;
	int virtualSize = getAssemblerBufferLength();
	int rawDataSize = roundToFileAlignment(virtualSize);
	int virtualAddress = sectionAlignment;

	bufferSeek(328); // Section headers start
	writeAscii64(".text");
	write32(virtualSize);
	write32(virtualAddress);
	write32(rawDataSize);
	write32(rawDataPos);
	bufferAdvance(12);
	write32(0x60000020 | 0x40000040);

	bufferSeek(rawDataPos);
	writeBytes(getAssemblerBuffer(), rawDataSize);
}



void eyreLink() {
	writeHeaders();
	writeSections();
}



void* getLinkerBuffer() {
	return buffer;
}



int getLinkerBufferLength() {
	return (int) (bufferPos - (void*) buffer);
}