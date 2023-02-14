#include "gen.h"
#include "internal.h"



// Variables



static char* path;
static char* chars;
static int size;
static int pos = 0;
static int lineNumber = 1;

#define ENCODINGS_CAPACITY 1024
static int encodingCount;
static EyreGenEncoding encodings[ENCODINGS_CAPACITY];

#define GROUPS_CAPACITY 512
static int groupCount;
static EyreGenGroup groups[GROUPS_CAPACITY];



// Errors



static void encodingErrorAt(char* format, char* file, int line, ...) {
	fprintf(stdout, "Encoding error at %s:%d: ", path, lineNumber);
	va_list args;
	va_start(args, line);
	vfprintf(stdout, format, args);
	fprintf(stdout, "\n");
	errorAt("Encoding error", file, line);
}

#define encodingError(format, ...) encodingErrorAt(format, __FILE__, __LINE__, ##__VA_ARGS__)



// Parsing utils



static void skipSpaces() {
	while(chars[pos] == ' ') pos++;
}



static void skipLine() {
	while(chars[pos] != '\n') pos++;
}



static int atNewline() {
	return chars[pos] == '\r' || chars[pos] == '\n';
}



static int isWhitespace(char c) {
	return c == ' ' || c == '\r' || c == '\n' || c == '\t';
}



// Integer parsing



static int parseHex(unsigned char c) {
	if(c >= '0' && c <= '9')
		return c - '0';
	if(c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if(c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	encodingError("Invalid hex: %d", c);
	return 0;
}



static int parseHexString(char* string, int length) {
	int value = 0;
	for(int i = 0; i < length; i++)
		value = (value << 4) | parseHex(string[i]);
	return value;
}



// String parsing



static int stringLength() {
	char* string = &chars[pos];
	int length = 0;
	while(1) {
		char c = string[length];
		if(isWhitespace(c)) break;
		length++;
	}
	return length;
}



static char* parseString(int length) {
	char* string = &chars[pos];
	pos += length;
	return string;
}



// Struct creation



static void addEncoding(EyreGenGroup* group, int opcode, int extension, EyreOperands operands, int widths) {
	if(encodingCount >= ENCODINGS_CAPACITY)
		encodingError("Too many encodings");

	for(int i = 0; i < group->encodingCount; i++)
		if(group->encodings[i]->operands == operands)
			return;

	EyreGenEncoding* encoding  = &encodings[encodingCount++];
	encoding->mnemonic  = group->mnemonic;
	encoding->opcode    = opcode;
	encoding->extension = extension;
	encoding->operands  = operands;
	encoding->widths    = widths;

	if(group->encodingCount >= 32)
		encodingError("Too many encodings for mnemonic %s", group->mnemonic);

	group->encodings[group->encodingCount++] = encoding;
	group->operandsBits |= (1 << encoding->operands);
	group->specifierBits |= (1 << eyreOperandsSpecifiers[encoding->operands]);
}



static EyreGenGroup* addGroup(char* string, int length) {
	for(int i = 0; i < groupCount; i++)
		if(length == strlen(groups[i].mnemonic) && memcmp(string, groups[i].mnemonic, length) == 0)
			return &groups[i];

	if(groupCount >= GROUPS_CAPACITY)
		encodingError("Too many groups");

	EyreGenGroup* group = &groups[groupCount++];
	memcpy(group->mnemonic, string, length);
	return group;
}



// Parsing



static int parseOperands(char* string, int length) {
	for(int i = 0; i < OPERANDS_COUNT; i++)
		if(length == strlen(eyreOperandsNames[i]) && memcmp(string, eyreOperandsNames[i], length) == 0)
			return i;

	return -1;
}



static int parseCompound(char* string, int length) {
	for(int i = 0; i < COMPOUND_COUNT; i++)
		if(length == strlen(eyreCompoundOperandsNames[i]) && memcmp(string, eyreCompoundOperandsNames[i], length) == 0)
			return i;

	return -1;
}



static int parseOpcode() {
	char* opcodeString = &chars[pos];
	int opcodeLength = 0;
	while(opcodeString[opcodeLength] != ' ' && opcodeString[opcodeLength] != '/') opcodeLength++;
	pos += opcodeLength;
	return parseHexString(opcodeString, opcodeLength);
}



static int parseExtension() {
	if(chars[pos] != '/') return 0;
	pos++;
	int extension = chars[pos++] - '0';
	if(extension < 0 || extension > 9)
		encodingError("Invalid extension: %d", extension);
	return extension;
}



static EyreGenGroup* parseGroup() {
	int mnemonicLength = stringLength();
	char* mnemonicString = parseString(mnemonicLength);
	return addGroup(mnemonicString, mnemonicLength);
}



static int parseWidths() {
	if(atNewline()) return 0;
	if(chars[pos] != '0' && chars[pos] != '1') return 0;
	int widths = 0;
	for(int i = 0; i < 4; i++) {
		char c = chars[pos];
		if(c == '1')
			widths |= (1 << i);
		else if(c != '0')
			encodingError("Invalid widths char: %d", c);
		pos++;
	}
	return widths;
}



static int parseCustom() {
	if(atNewline()) return 0;

	if(memcmp(&chars[pos], "CUSTOM1", 7) == 0) {
		pos += 7;
		return OPERANDS_CUSTOM1;
	} else if(memcmp(&chars[pos], "CUSTOM2", 7) == 0) {
		pos += 7;
		return OPERANDS_CUSTOM2;
	} else {
		return 0;
	}
}



// Parsing



static void parseEncodings(char* inputPath) {
	path = inputPath;
	readFile(path);

	chars = getReadFileData();
	size = getReadFileLength();

	while(pos < size) {
		char c = chars[pos];

		if(c == ';') break;

		if(c == '\n') {
			lineNumber++;
			pos++;
			continue;
		}

		if(c == ' ' || c == '\t' || c == '\r') {
			pos++;
			continue;
		}

		if(c == '#') {
			skipLine();
			continue;
		}

		int opcode = parseOpcode();
		int extension = parseExtension();
		skipSpaces();

		EyreGenGroup* group = parseGroup();
		skipSpaces();

		int operandsLength = stringLength();
		char* operandsString = parseString(operandsLength);
		int operands = parseOperands(operandsString, operandsLength);
		int compound = parseCompound(operandsString, operandsLength);
		skipSpaces();

		int widths = parseWidths();
		skipSpaces();
		int custom = parseCustom();
		skipLine();

		if(custom) operands = custom;

		if(compound >= 0) {
			EyreOperands* compoundOperands = eyreCompoundOperandsMap[compound];
			for(int i = 0; i < 5; i++) {
				if(compoundOperands[i] == 0) break;
				addEncoding(group, opcode, extension, compoundOperands[i], widths);
			}
		} else if(operands >= 0) {
			addEncoding(group, opcode, extension, operands, widths);
		} else if(operandsLength == 0) {
			addEncoding(group, opcode, extension, OPERANDS_NONE, widths);
		} else {
			encodingError("Invalid operands: %.*s", operandsLength, operandsString);
		}
	}

	for(int i = 0; i < groupCount; i++) {
		EyreGenEncoding** list = groups[i].encodings;
		int count = groups[i].encodingCount;

		while(1) {
			int comparisons = 0;

			for(int j = 0; j < count - 1; j++) {
				EyreGenEncoding* a = list[j];
				EyreGenEncoding* b = list[j + 1];
				if(b->operands < a->operands) {
					if(b->operands == OPERANDS_CUSTOM2) printInt(1);
					list[j] = b;
					list[j + 1] = a;
					comparisons++;
				}
			}

			if(comparisons == 0) break;
		}
	}
}



// Generation



static char buffer[100000];

static int bufferSize = 0;


void print(char* format, ...) {
	va_list list;
	va_start(list, format);
	int count = vsprintf(buffer + bufferSize, format, list);
	bufferSize += count;
}



static void genGroups() {
	bufferSize = 0;
	
	print("#include \"encodings.h\"\n\n");

	for(int i = 0; i < groupCount; i++) {
		EyreGenGroup g = groups[i];

		print("static EyreEncoding EYRE_ENCODINGS_%s[] = {\n", g.mnemonic);
		for(int j = 0; j < g.encodingCount; j++) {
			EyreGenEncoding* e = g.encodings[j];
			print("\t{ %d, %d, %d, %d },\n", e->opcode, e->extension, 0, e->widths);
		}
		print("};\n\n");
	}

	print("static EyreGroup eyreEncodings[] = {\n");
	for(int i = 0; i < groupCount; i++) {
		EyreGenGroup g = groups[i];
		print("\t{ %d, %d, EYRE_ENCODINGS_%s },\n", g.operandsBits, g.specifierBits, g.mnemonic);
	}
	print("};\n");

	print("\nEyreGroup* eyreGetEncodings(int mnemonic) { return &eyreEncodings[mnemonic]; }\n\n\n");

	copyFile(getLocalFile("src/encodings.c"), getLocalFile("prev/encodings.c"));
	writeFile(getLocalFile("src/encodings.c"), bufferSize, buffer);
}



static void genMnemonicsHeader() {
	bufferSize = 0;
	print("#ifndef INCLUDE_MNEMONICS\n");
	print("#define INCLUDE_MNEMONICS\n\n");
	print("typedef enum EyreMnemonic {\n");
	for(int i = 0; i < groupCount; i++) {
		if(i % 4 == 0) print("\t");
		print("MNEMONIC_%s, ", groups[i].mnemonic);
		if(i % 4 == 3) print("\n");
	}
	if(groupCount % 4 != 3) print("\n");
	print("\tMNEMONIC_COUNT\n");
	print("} EyreMnemonic;\n\n");
	print("extern char* eyreMnemonicNames[MNEMONIC_COUNT];\n\n");
	print("#endif\n");

	copyFile(getLocalFile("src/mnemonics.h"), getLocalFile("prev/mnemonics.h"));
	writeFile(getLocalFile("src/mnemonics.h"), bufferSize, buffer);
}



static void genMnemonics() {
	bufferSize = 0;
	print("#include \"mnemonics.h\"\n\n");
	print("char* eyreMnemonicNames[MNEMONIC_COUNT] = {\n");
	for(int i = 0; i < groupCount; i++) {
		char lowercase[16];
		for(int j = 0; j < 16; j++) {
			char c = groups[i].mnemonic[j];
			if(c >= 'A' && c <= 'Z') lowercase[j] = c - 'A' + 'a'; else lowercase[j] = c;
		}
		if(i % 4 == 0) print("\t");
		print("\"%s\", ", lowercase);
		if(i % 4 == 3) print("\n");
	}
	if(groupCount % 4 != 3) print("\n");
	print("};\n");

	copyFile(getLocalFile("src/mnemonics.c"), getLocalFile("prev/mnemonics.c"));
	writeFile(getLocalFile("src/mnemonics.c"), bufferSize, buffer);
}



void eyreGen(char* inputPath) {
	parseEncodings(inputPath);
	createDirectory(getLocalFile("prev"));
	genGroups();
	genMnemonicsHeader();
	genMnemonics();
}



/*
static void printEncodings() {
	for(int i = 0; i < encodingCount; i++) {
		EyreGenEncoding encoding = encodings[i];
		printf("%x/%d %s", encoding.opcode, encoding.extension, encoding.mnemonic);
		if(encoding.operands >= 0)
			printf(" %s", eyreOperandsNames[encoding.operands]);
		if(encoding.widths > 0)
			printf(" %d", encoding.widths);
		printf("\n");
	}
}*/
