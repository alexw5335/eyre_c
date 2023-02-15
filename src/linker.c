#include "internal.h"
#include "buffer.h"
#include "nodes.h"
#include "enums.h"
#include "symbols.h"
#include "intern.h"




#define sectionAlignment 0x1000
#define fileAlignment 0x200

const int numSectionsPos = 70;
const int entryPointPos = 104;
const int imageSizePos = 144;
const int idataDirPos = 208;
const int sectionHeadersPos = 328;

static int linkLength;
static int nextSectionRva;
static int nextSectionPos;

static int sectionLengths[SECTION_COUNT];
static int sectionAddresses[SECTION_COUNT];
static int sectionPositions[SECTION_COUNT];
static int numSections = 0;

static int getSectionPos(EyreSection section) {
	if(sectionPositions[section] == 0)
		error("Uninitialised section: %d", section);
	return sectionPositions[section];
}

static int getSectionAddress(EyreSection section) {
	if(sectionAddresses[section] == 0)
		error("uninitialised section: %d", section);
	return sectionAddresses[section];
}

static int getSymbolAddress(PosSymbol* symbol) {
	return getSectionAddress(symbol->section) + symbol->pos;
}


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
	write32(0);          // pEntryPoint    (fill in later);
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

	nextSectionPos = fileAlignment;
	nextSectionRva = sectionAlignment;

}



static void writeSection(
	char*       name,
	int         characteristics,
	void*       bytes,
	int         size,
	int         extraSize,
	EyreSection section
) {
	int virtualAddress = nextSectionRva; // Must be aligned to sectionAlignment
	int rawDataPos     = nextSectionPos; // Must be aligned to fileAlignment
	int rawDataSize    = roundToFileAlignment(size); // Must be aligned to fileAlignment
	int virtualSize    = size + extraSize; // No alignment requirement, may be smaller than rawDataSize

	bufferSeek(rawDataPos);
	if(bytes != NULL)
		writeBytes(bytes, size);
	else
		bufferPos += size;

	nextSectionPos += rawDataSize;
	nextSectionRva += roundToSectionAlignment(virtualSize);

	zeroTo(nextSectionPos);

	bufferPos = sectionHeadersPos + numSections * 40;
	numSections++;

	writeAscii64(name);
	write32(virtualSize);
	write32(virtualAddress);
	write32(rawDataSize);
	write32(rawDataPos);
	writeZero(12);
	write32(characteristics);

	bufferPos = nextSectionPos;

	sectionAddresses[section] = virtualAddress;
	sectionPositions[section] = rawDataPos;
	sectionLengths[section] = virtualSize;
}



static void writeSections() {
	writeSection(
		".text",
		0x60000020,
		getAssemblerBuffer(),
		getAssemblerBufferLength(),
		0,
		SECTION_TEXT
	);
}



// Relocations



static int resolveImmRec(void* n, int regValid) {
	char type = *(char*) n;

	if(type == NODE_INT) {
		IntNode* node = n;
		return node->value;
	}

	if(type == NODE_UNARY) {
		UnaryNode* node = n;
		return calcUnaryInt(
			node->op,
			resolveImmRec(node->value, regValid & (node->op == UNARY_POS))
		);
	}

	if(type == NODE_BINARY) {
		BinaryNode* node = n;
		return calcBinaryInt(
			node->op,
			resolveImmRec(node->left, regValid & ((node->op == BINARY_ADD) | (node->op == BINARY_SUB))),
			resolveImmRec(node->right, regValid & (node->op == BINARY_ADD))
		);
	}

	if(type == NODE_SYM) {
		SymNode* node = n;
		SymBase* symBase = node->symbol;

		if(symBase->flags & SYM_FLAGS_POS) {
			PosSymbol* posSym = (PosSymbol*) symBase;
			return getSectionAddress(posSym->section) + posSym->pos;
		} else {
			error("Invalid symbol");
		}
	}

	error("Invalid node: %d", type);

	return 0;
}



static int resolveImm(void* n) {
	if(nodeType(n) == NODE_IMM)
		n = ((ImmNode*)n)->value;
	return resolveImmRec(n, 1);
}



static void writeRelocations() {
	for(int i = 0; i < relocationCount; i++) {
		Relocation relocation = relocations[i];

		if(relocation.offset == -2) {
			int value = resolveImm(relocation.node);
			bufferSeek(getSectionPos(relocation.section) + relocation.pos);
			if(!writeWidth(relocation.width, value))
				error("Invalid relocation");
		} else if(relocation.offset >= 0) {
			int value = resolveImm(relocation.node);
			int base = getSectionAddress(relocation.section) + relocation.pos + (1 << relocation.width) + relocation.offset;
			value -= base;
			bufferSeek(getSectionPos(relocation.section) + relocation.pos);
			if(!writeWidth(relocation.width, value))
				error("Invalid relocation");
		} else {
			error("Invalid relocation");
		}

	}
}



static void writeImports() {
	if(dllImportCount == 0) return;

	int idtsRva = nextSectionRva;
	int idtsPos = bufferPos;
	int idtsSize = dllImportCount * 20 + 20;
	int offset = idtsPos - idtsRva;

	write32At(idataDirPos, idtsRva);
	write32At(idataDirPos + 4, dllImportCount * 20 + 20);
	writeZero(idtsSize);

	for(int dllIndex = 0; dllIndex < dllImportCount; dllIndex++) {
		DllImport dll = dllImports[dllIndex];

		int idtPos = idtsPos + dllIndex * 20;
		int dllNamePos = bufferPos;

		writeAscii(eyreGetString(dll.dllName)->data);
		writeAscii(".dll");
		write8(0);
		bufferPos = (bufferPos + 7) & -8; // align by 8, not necessary?

		int iltPos = bufferPos;

		writeZero(dll.importCount * 8 + 8);

		int iatPos = bufferPos;

		writeZero(dll.importCount * 8 + 8);

		for(int importIndex = 0; importIndex < dll.importCount; importIndex++) {
			DllImportSymbol* import = dll.imports[importIndex];
			write32At(iltPos + importIndex * 8, bufferPos - offset);
			write32At(iatPos + importIndex * 8, bufferPos - offset);

			write16(0);
			writeAsciiNt(eyreGetString(import->importName)->data);
			if(bufferPos & 1) bufferPos++; // align even
			import->pos = iatPos + importIndex * 8 - idtsPos;
		}

		write32At(idtPos, iltPos - offset);
		write32At(idtPos + 12, dllNamePos - offset);
		write32At(idtPos + 16, iatPos - offset);
	}

	int size = bufferPos - idtsPos;

	writeSection(
		".idata",
		0x40000040,
		NULL,
		size,
		0,
		SECTION_IDATA
	);
}



void eyreLink() {
	writeHeaders();
	writeSections();
	writeImports();
	linkLength = bufferPos;
	writeRelocations();

	if(entryPoint != NULL) {
		bufferSeek(entryPointPos);
		write32(getSymbolAddress(entryPoint));
	}

	bufferSeek(imageSizePos);
	write32(nextSectionRva);

	bufferSeek(numSectionsPos);
	write32(numSections);
}



void* getLinkerBuffer() {
	return buffer;
}



void* getTextSectionBuffer() {
	return &buffer[sectionPositions[SECTION_TEXT]];
}



int getTextSectionLength() {
	return sectionLengths[SECTION_TEXT];
}



int getLinkerBufferLength() {
	return linkLength;
}