#include "eyre.h"

int main() {
	eyreInitInterns();
	SrcFile file;
	eyreCreateSrcFile(&file, eyreGetLocalFile("test.txt"));
	eyreLex(&file);
	eyrePrintTokens();
	eyreParse();
	eyrePrintNodes();
}