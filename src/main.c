#include "internal.h"
#include "gen.h"
#include "intern.h"



static void test() {
	eyreInitInterns();
	SrcFile srcFile = { .path = getLocalFile("test.eyre") };
	eyreLex(&srcFile);
	//eyrePrintTokens(&srcFile);
	//printNewline();
	eyreParse(&srcFile);
	eyrePrintNodes(&srcFile);
	eyreResolve(&srcFile);
	printNewline();
	eyreAssemble(&srcFile);

	eyreLink();
	writeFile(getLocalFile("test.exe"), getLinkerBufferLength(), getLinkerBuffer());
	runCommandArgs(3, "dumpbin", "/all", getLocalFile("test.exe"));

	writeFile(getLocalFile("test.obj"), getTextSectionLength(), getTextSectionBuffer());
	runCommandArgs(3, "ndisasm", "-b64", getLocalFile("test.obj"));
}




int main() {
	test();
	//eyreGen(getLocalFile("encodings.txt"));
}