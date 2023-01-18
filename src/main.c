#include "eyre.h"

int main() {
	SrcFile file;
	eyreCreateSrcFileFromFile(&file, eyreGetFileInCurrentDirectory("test.txt"));
	eyreLex(&file);
	eyrePrintTokens();
}