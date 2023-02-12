#include "internal.h"
#include "gen.h"
#include "intern.h"


static void gen() {
	eyreParseEncodings("encodings.txt");
	//eyreGenGroups();
	eyreGenMnemonics();
}



static void test() {
	eyreInitInterns();
	SrcFile srcFile = { .path = getLocalFile("test.eyre") };
	eyreLex(&srcFile);
	eyrePrintTokens(&srcFile);
	printNewline();
	eyreParse(&srcFile);
	eyrePrintNodes(&srcFile);
	eyreResolve(&srcFile);
	printNewline();
	eyreAssemble(&srcFile);
	writeFile(getLocalFile("test.obj"), getAssemblerBufferLength(), getAssemblerBuffer());
	runCommandArgs(3, "ndisasm", "-b64", getLocalFile("test.obj"));
	//eyreLink();
	//writeFile(getLocalFile("test.exe"), getLinkerBufferLength(), getLinkerBuffer());
}



int main() {
	test();
}