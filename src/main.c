#include "eyre.h"
#include "gen.h"
#include "intern.h"
#include "parse.h"
#include "resolve.h"



void test() {
	eyreInitInterns();
	SrcFile file;
	eyreCreateSrcFile(&file, eyreGetLocalFile("test.txt"));
	eyreLex(&file);
	//eyrePrintTokens();
	//printNewline();
	eyreParse(&file);
	eyrePrintNodes();
	printNewline();
	eyrePrintSymbols();
	printNewline();
	eyreResolve(&file);
	eyreAssemble(&file);
}



void gen() {
	eyreParseEncodings("encodings.txt");
	eyreGenGroups();
}



int main() {
	test();
}