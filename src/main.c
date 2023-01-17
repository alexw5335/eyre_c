#include "eyre.h"

int main() {
	SrcFile file;
	createSrcFile(&file, getFileInCurrentDirectory("test.txt"));
	eyreLex(&file);
	printTokens();
}