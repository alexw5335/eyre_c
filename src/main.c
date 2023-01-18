#include "eyre.h"

int main() {
	eyreInitInterns();
	SrcFile file;
	eyreCreateSrcFileFromFile(&file, eyreGetFileInCurrentDirectory("test.txt"));
	eyreLex(&file);
	eyrePrintTokens();
}