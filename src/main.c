#include "eyre.h"
#include "gen.h"
#include "intern.h"



void test() {
	eyreInitInterns();
	SrcFile file;
	eyreCreateSrcFile(&file, eyreGetLocalFile("test.txt"));
	eyreLex(&file);
	eyrePrintTokens();
	eyreParse(&file);
	eyrePrintNodes();
}



void gen() {
	eyreParseEncodings("encodings.txt");
	eyreGenGroups();
}



int main() {
	test();
}