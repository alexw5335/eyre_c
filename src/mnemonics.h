#ifndef INCLUDE_MNEMONICS
#define INCLUDE_MNEMONICS

typedef enum EyreMnemonic {
	MNEMONIC_ADD, MNEMONIC_OR, MNEMONIC_ADC, MNEMONIC_SBB, 
	MNEMONIC_AND, MNEMONIC_SUB, MNEMONIC_XOR, MNEMONIC_CMP, 
	MNEMONIC_PUSH, MNEMONIC_POP, MNEMONIC_MOVSXD, MNEMONIC_MOVSX, 
	MNEMONIC_MOVZX, MNEMONIC_INSB, MNEMONIC_INSW, MNEMONIC_INSD, 
	MNEMONIC_OUTSB, MNEMONIC_OUTSW, MNEMONIC_OUTSD, MNEMONIC_JA, 
	MNEMONIC_JAE, MNEMONIC_JB, MNEMONIC_JBE, MNEMONIC_JC, 
	MNEMONIC_JE, MNEMONIC_JG, MNEMONIC_JGE, MNEMONIC_JL, 
	MNEMONIC_JLE, MNEMONIC_JNA, MNEMONIC_JNAE, MNEMONIC_JNB, 
	MNEMONIC_JNBE, MNEMONIC_JNC, MNEMONIC_JNE, MNEMONIC_JNG, 
	MNEMONIC_JNGE, MNEMONIC_JNL, MNEMONIC_JNLE, MNEMONIC_JNO, 
	MNEMONIC_JNP, MNEMONIC_JNS, MNEMONIC_JNZ, MNEMONIC_JO, 
	MNEMONIC_JP, MNEMONIC_JPE, MNEMONIC_JPO, MNEMONIC_JS, 
	MNEMONIC_JZ, MNEMONIC_TEST, MNEMONIC_XCHG, MNEMONIC_MOV, 
	MNEMONIC_LEA, MNEMONIC_NOP, MNEMONIC_CBW, MNEMONIC_CWDE, 
	MNEMONIC_CDQE, MNEMONIC_CWD, MNEMONIC_CDQ, MNEMONIC_CQO, 
	MNEMONIC_WAIT, MNEMONIC_FWAIT, MNEMONIC_PUSHF, MNEMONIC_PUSHFQ, 
	MNEMONIC_LAHF, MNEMONIC_MOVSB, MNEMONIC_MOVSW, MNEMONIC_MOVSD, 
	MNEMONIC_MOVSQ, MNEMONIC_CMPSB, MNEMONIC_CMPSW, MNEMONIC_CMPSD, 
	MNEMONIC_CMPSQ, MNEMONIC_STOSB, MNEMONIC_STOSW, MNEMONIC_STOSD, 
	MNEMONIC_STOSQ, MNEMONIC_SCASB, MNEMONIC_SCASW, MNEMONIC_SCASD, 
	MNEMONIC_SCASQ, MNEMONIC_LODSB, MNEMONIC_LODSW, MNEMONIC_LODSD, 
	MNEMONIC_LODSQ, MNEMONIC_ROL, MNEMONIC_ROR, MNEMONIC_RCL, 
	MNEMONIC_RCR, MNEMONIC_SAL, MNEMONIC_SHL, MNEMONIC_SHR, 
	MNEMONIC_SAR, MNEMONIC_RET, MNEMONIC_RETF, MNEMONIC_LEAVEW, 
	MNEMONIC_LEAVE, MNEMONIC_INT3, MNEMONIC_INT, MNEMONIC_INT1, 
	MNEMONIC_IRETW, MNEMONIC_IRETD, MNEMONIC_IRETQ, MNEMONIC_JECXZ, 
	MNEMONIC_JRCXZ, MNEMONIC_IN, MNEMONIC_OUT, MNEMONIC_HLT, 
	MNEMONIC_CMC, MNEMONIC_NOT, MNEMONIC_NEG, MNEMONIC_MUL, 
	MNEMONIC_IMUL, MNEMONIC_DIV, MNEMONIC_IDIV, MNEMONIC_CLC, 
	MNEMONIC_STC, MNEMONIC_CLI, MNEMONIC_STI, MNEMONIC_CLD, 
	MNEMONIC_STD, MNEMONIC_INC, MNEMONIC_DEC, MNEMONIC_CALL, 
	MNEMONIC_CALLF, MNEMONIC_JMP, MNEMONIC_JMPF, MNEMONIC_RDRAND, 
	MNEMONIC_RDSEED, 
	MNEMONIC_COUNT
} EyreMnemonic;

extern char* eyreMnemonicNames[MNEMONIC_COUNT];

#endif
