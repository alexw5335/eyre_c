#include "eyre.h"
#include "gen.h"
#include "intern.h"
#include "parse.h"
#include "resolve.h"
#include "assemble.h"



void test() {
	eyreInitInterns();
	SrcFile file;
	eyreCreateSrcFile(&file, eyreGetLocalFile("test.eyre"));
	eyreLex(&file);
	eyrePrintTokens();
	printNewline();
	eyreParse(&file);
	eyrePrintNodes();
	printNewline();
	eyrePrintSymbols();
	printNewline();
	//eyreResolve(&file);
	//eyreAssemble(&file);
	//eyreWriteFile(eyreGetLocalFile("test.obj"), eyreGetAssemblerBufferSize(), eyreGetAssemblerBuffer());
	//eyreRunCommandArgs(3, "ndisasm", "-b64", eyreGetLocalFile("test.obj"));
}



void gen() {
	eyreParseEncodings("encodings.txt");
	//eyreGenGroups();
	eyreGenMnemonics();
}



int main() {
	test();
}