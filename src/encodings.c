#include "encodings.h"

static EyreEncoding EYRE_ENCODINGS_ADD[] = {
	{ 0, 0, 0, 15 },
	{ 2, 0, 0, 15 },
	{ 2, 0, 0, 15 },
	{ 0, 0, 0, 15 },
	{ 128, 0, 0, 15 },
	{ 128, 0, 0, 15 },
	{ 131, 0, 0, 14 },
	{ 131, 0, 0, 14 },
	{ 4, 0, 0, 15 },
};

static EyreEncoding EYRE_ENCODINGS_OR[] = {
	{ 8, 0, 0, 15 },
	{ 10, 0, 0, 15 },
	{ 10, 0, 0, 15 },
	{ 8, 0, 0, 15 },
	{ 128, 1, 0, 15 },
	{ 128, 1, 0, 15 },
	{ 131, 1, 0, 14 },
	{ 131, 1, 0, 14 },
	{ 12, 0, 0, 15 },
};

static EyreEncoding EYRE_ENCODINGS_ADC[] = {
	{ 16, 0, 0, 15 },
	{ 18, 0, 0, 15 },
	{ 18, 0, 0, 15 },
	{ 16, 0, 0, 15 },
	{ 128, 2, 0, 15 },
	{ 128, 2, 0, 15 },
	{ 131, 2, 0, 14 },
	{ 131, 2, 0, 14 },
	{ 20, 0, 0, 15 },
};

static EyreEncoding EYRE_ENCODINGS_SBB[] = {
	{ 24, 0, 0, 15 },
	{ 26, 0, 0, 15 },
	{ 26, 0, 0, 15 },
	{ 24, 0, 0, 15 },
	{ 128, 3, 0, 15 },
	{ 128, 3, 0, 15 },
	{ 131, 3, 0, 14 },
	{ 131, 3, 0, 14 },
	{ 28, 0, 0, 15 },
};

static EyreEncoding EYRE_ENCODINGS_AND[] = {
	{ 32, 0, 0, 15 },
	{ 34, 0, 0, 15 },
	{ 34, 0, 0, 15 },
	{ 32, 0, 0, 15 },
	{ 128, 4, 0, 15 },
	{ 128, 4, 0, 15 },
	{ 131, 4, 0, 14 },
	{ 131, 4, 0, 14 },
	{ 36, 0, 0, 15 },
};

static EyreEncoding EYRE_ENCODINGS_SUB[] = {
	{ 40, 0, 0, 15 },
	{ 42, 0, 0, 15 },
	{ 42, 0, 0, 15 },
	{ 40, 0, 0, 15 },
	{ 128, 5, 0, 15 },
	{ 128, 5, 0, 15 },
	{ 131, 5, 0, 14 },
	{ 131, 5, 0, 14 },
	{ 44, 0, 0, 15 },
};

static EyreEncoding EYRE_ENCODINGS_XOR[] = {
	{ 48, 0, 0, 15 },
	{ 50, 0, 0, 15 },
	{ 50, 0, 0, 15 },
	{ 48, 0, 0, 15 },
	{ 128, 6, 0, 15 },
	{ 128, 6, 0, 15 },
	{ 131, 6, 0, 14 },
	{ 131, 6, 0, 14 },
	{ 52, 0, 0, 15 },
};

static EyreEncoding EYRE_ENCODINGS_CMP[] = {
	{ 56, 0, 0, 15 },
	{ 58, 0, 0, 15 },
	{ 58, 0, 0, 15 },
	{ 56, 0, 0, 15 },
	{ 128, 7, 0, 15 },
	{ 128, 7, 0, 15 },
	{ 131, 7, 0, 14 },
	{ 131, 7, 0, 14 },
	{ 60, 0, 0, 15 },
};

static EyreEncoding EYRE_ENCODINGS_PUSH[] = {
	{ 255, 6, 0, 14 },
	{ 80, 0, 0, 10 },
	{ 106, 0, 0, 0 },
	{ 104, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_POP[] = {
	{ 143, 0, 0, 14 },
	{ 88, 0, 0, 10 },
};

static EyreEncoding EYRE_ENCODINGS_MOVSXD[] = {
	{ 99, 0, 0, 14 },
};

static EyreEncoding EYRE_ENCODINGS_MOVSX[] = {
	{ 48655, 0, 0, 14 },
	{ 48911, 0, 0, 12 },
};

static EyreEncoding EYRE_ENCODINGS_MOVZX[] = {
	{ 46607, 0, 0, 14 },
	{ 46863, 0, 0, 12 },
};

static EyreEncoding EYRE_ENCODINGS_INSB[] = {
	{ 108, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_INSW[] = {
	{ 28006, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_INSD[] = {
	{ 109, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_OUTSB[] = {
	{ 110, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_OUTSW[] = {
	{ 28518, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_OUTSD[] = {
	{ 111, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_JA[] = {
	{ 119, 0, 0, 0 },
	{ 34575, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_JAE[] = {
	{ 115, 0, 0, 0 },
	{ 33551, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_JB[] = {
	{ 114, 0, 0, 0 },
	{ 33295, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_JBE[] = {
	{ 118, 0, 0, 0 },
	{ 34319, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_JC[] = {
	{ 114, 0, 0, 0 },
	{ 33295, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_JE[] = {
	{ 116, 0, 0, 0 },
	{ 33807, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_JG[] = {
	{ 127, 0, 0, 0 },
	{ 36623, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_JGE[] = {
	{ 125, 0, 0, 0 },
	{ 36111, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_JL[] = {
	{ 124, 0, 0, 0 },
	{ 35855, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_JLE[] = {
	{ 126, 0, 0, 0 },
	{ 36367, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_JNA[] = {
	{ 118, 0, 0, 0 },
	{ 34319, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_JNAE[] = {
	{ 114, 0, 0, 0 },
	{ 33295, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_JNB[] = {
	{ 115, 0, 0, 0 },
	{ 33551, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_JNBE[] = {
	{ 119, 0, 0, 0 },
	{ 34575, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_JNC[] = {
	{ 115, 0, 0, 0 },
	{ 33551, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_JNE[] = {
	{ 117, 0, 0, 0 },
	{ 34063, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_JNG[] = {
	{ 126, 0, 0, 0 },
	{ 36367, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_JNGE[] = {
	{ 124, 0, 0, 0 },
	{ 35855, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_JNL[] = {
	{ 125, 0, 0, 0 },
	{ 36111, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_JNLE[] = {
	{ 127, 0, 0, 0 },
	{ 36623, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_JNO[] = {
	{ 113, 0, 0, 0 },
	{ 33039, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_JNP[] = {
	{ 123, 0, 0, 0 },
	{ 35599, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_JNS[] = {
	{ 121, 0, 0, 0 },
	{ 35087, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_JNZ[] = {
	{ 117, 0, 0, 0 },
	{ 34063, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_JO[] = {
	{ 112, 0, 0, 0 },
	{ 32783, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_JP[] = {
	{ 122, 0, 0, 0 },
	{ 35343, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_JPE[] = {
	{ 122, 0, 0, 0 },
	{ 35343, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_JPO[] = {
	{ 123, 0, 0, 0 },
	{ 35599, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_JS[] = {
	{ 120, 0, 0, 0 },
	{ 34831, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_JZ[] = {
	{ 116, 0, 0, 0 },
	{ 33807, 0, 0, 0 },
	{ 33807, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_TEST[] = {
	{ 132, 0, 0, 15 },
	{ 132, 0, 0, 15 },
	{ 246, 0, 0, 15 },
	{ 246, 0, 0, 15 },
	{ 168, 0, 0, 15 },
};

static EyreEncoding EYRE_ENCODINGS_XCHG[] = {
	{ 134, 0, 0, 15 },
	{ 134, 0, 0, 15 },
	{ 134, 0, 0, 15 },
	{ 134, 0, 0, 15 },
	{ 144, 0, 0, 14 },
	{ 144, 0, 0, 14 },
};

static EyreEncoding EYRE_ENCODINGS_MOV[] = {
	{ 136, 0, 0, 15 },
	{ 138, 0, 0, 15 },
	{ 138, 0, 0, 15 },
	{ 136, 0, 0, 15 },
	{ 198, 0, 0, 15 },
	{ 198, 0, 0, 15 },
	{ 176, 0, 0, 15 },
};

static EyreEncoding EYRE_ENCODINGS_LEA[] = {
	{ 141, 0, 0, 14 },
};

static EyreEncoding EYRE_ENCODINGS_NOP[] = {
	{ 144, 0, 0, 0 },
	{ 7951, 0, 0, 6 },
	{ 7951, 0, 0, 6 },
};

static EyreEncoding EYRE_ENCODINGS_CBW[] = {
	{ 39014, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_CWDE[] = {
	{ 152, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_CDQE[] = {
	{ 38984, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_CWD[] = {
	{ 39270, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_CDQ[] = {
	{ 153, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_CQO[] = {
	{ 39240, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_WAIT[] = {
	{ 155, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_FWAIT[] = {
	{ 155, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_PUSHF[] = {
	{ 40038, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_PUSHFQ[] = {
	{ 40008, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_LAHF[] = {
	{ 159, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_MOVSB[] = {
	{ 164, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_MOVSW[] = {
	{ 42342, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_MOVSD[] = {
	{ 165, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_MOVSQ[] = {
	{ 42312, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_CMPSB[] = {
	{ 166, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_CMPSW[] = {
	{ 42854, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_CMPSD[] = {
	{ 167, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_CMPSQ[] = {
	{ 42824, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_STOSB[] = {
	{ 170, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_STOSW[] = {
	{ 43878, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_STOSD[] = {
	{ 171, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_STOSQ[] = {
	{ 43848, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_SCASB[] = {
	{ 174, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_SCASW[] = {
	{ 44902, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_SCASD[] = {
	{ 175, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_SCASQ[] = {
	{ 44872, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_LODSB[] = {
	{ 172, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_LODSW[] = {
	{ 44390, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_LODSD[] = {
	{ 173, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_LODSQ[] = {
	{ 44360, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_ROL[] = {
	{ 192, 0, 0, 15 },
	{ 192, 0, 0, 15 },
	{ 208, 0, 0, 15 },
	{ 210, 0, 0, 15 },
};

static EyreEncoding EYRE_ENCODINGS_ROR[] = {
	{ 192, 1, 0, 15 },
	{ 192, 1, 0, 15 },
	{ 208, 1, 0, 15 },
	{ 210, 1, 0, 15 },
};

static EyreEncoding EYRE_ENCODINGS_RCL[] = {
	{ 192, 2, 0, 15 },
	{ 192, 2, 0, 15 },
	{ 208, 2, 0, 15 },
	{ 210, 2, 0, 15 },
};

static EyreEncoding EYRE_ENCODINGS_RCR[] = {
	{ 192, 3, 0, 15 },
	{ 192, 3, 0, 15 },
	{ 208, 3, 0, 15 },
	{ 210, 3, 0, 15 },
};

static EyreEncoding EYRE_ENCODINGS_SAL[] = {
	{ 192, 4, 0, 15 },
	{ 192, 4, 0, 15 },
	{ 208, 4, 0, 15 },
	{ 210, 4, 0, 15 },
};

static EyreEncoding EYRE_ENCODINGS_SHL[] = {
	{ 192, 4, 0, 15 },
	{ 192, 4, 0, 15 },
	{ 208, 4, 0, 15 },
	{ 210, 4, 0, 15 },
};

static EyreEncoding EYRE_ENCODINGS_SHR[] = {
	{ 192, 5, 0, 15 },
	{ 192, 5, 0, 15 },
	{ 208, 5, 0, 15 },
	{ 210, 5, 0, 15 },
};

static EyreEncoding EYRE_ENCODINGS_SAR[] = {
	{ 192, 7, 0, 15 },
	{ 192, 7, 0, 15 },
	{ 208, 7, 0, 15 },
	{ 210, 7, 0, 15 },
};

static EyreEncoding EYRE_ENCODINGS_RET[] = {
	{ 195, 0, 0, 0 },
	{ 194, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_RETF[] = {
	{ 203, 0, 0, 0 },
	{ 202, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_LEAVEW[] = {
	{ 51558, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_LEAVE[] = {
	{ 201, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_INT3[] = {
	{ 204, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_INT[] = {
	{ 205, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_INT1[] = {
	{ 241, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_IRETW[] = {
	{ 53094, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_IRETD[] = {
	{ 207, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_IRETQ[] = {
	{ 53064, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_JCXZ[] = {
	{ 227, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_JRCXZ[] = {
	{ 58184, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_IN[] = {
	{ 228, 0, 0, 7 },
	{ 236, 0, 0, 7 },
};

static EyreEncoding EYRE_ENCODINGS_OUT[] = {
	{ 230, 0, 0, 7 },
	{ 238, 0, 0, 7 },
};

static EyreEncoding EYRE_ENCODINGS_HLT[] = {
	{ 244, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_CMC[] = {
	{ 245, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_NOT[] = {
	{ 246, 2, 0, 15 },
	{ 246, 2, 0, 15 },
};

static EyreEncoding EYRE_ENCODINGS_NEG[] = {
	{ 246, 3, 0, 15 },
	{ 246, 3, 0, 15 },
};

static EyreEncoding EYRE_ENCODINGS_MUL[] = {
	{ 246, 4, 0, 15 },
	{ 246, 4, 0, 15 },
};

static EyreEncoding EYRE_ENCODINGS_IMUL[] = {
	{ 246, 5, 0, 15 },
	{ 246, 5, 0, 15 },
	{ 44815, 0, 0, 14 },
	{ 44815, 0, 0, 14 },
	{ 107, 0, 0, 14 },
	{ 105, 0, 0, 14 },
};

static EyreEncoding EYRE_ENCODINGS_DIV[] = {
	{ 246, 6, 0, 15 },
	{ 246, 6, 0, 15 },
};

static EyreEncoding EYRE_ENCODINGS_IDIV[] = {
	{ 247, 7, 0, 15 },
	{ 247, 7, 0, 15 },
};

static EyreEncoding EYRE_ENCODINGS_CLC[] = {
	{ 248, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_STC[] = {
	{ 249, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_CLI[] = {
	{ 250, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_STI[] = {
	{ 251, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_CLD[] = {
	{ 252, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_STD[] = {
	{ 253, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_INC[] = {
	{ 254, 0, 0, 15 },
	{ 254, 0, 0, 15 },
};

static EyreEncoding EYRE_ENCODINGS_DEC[] = {
	{ 254, 1, 0, 15 },
	{ 254, 1, 0, 15 },
};

static EyreEncoding EYRE_ENCODINGS_CALL[] = {
	{ 255, 2, 0, 8 },
	{ 255, 2, 0, 8 },
	{ 232, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_CALLF[] = {
	{ 255, 3, 0, 14 },
};

static EyreEncoding EYRE_ENCODINGS_JMP[] = {
	{ 255, 4, 0, 8 },
	{ 255, 4, 0, 8 },
	{ 235, 0, 0, 0 },
	{ 233, 0, 0, 0 },
};

static EyreEncoding EYRE_ENCODINGS_JMPF[] = {
	{ 255, 5, 0, 14 },
};

static EyreEncoding EYRE_ENCODINGS_RDRAND[] = {
	{ 50959, 6, 0, 14 },
};

static EyreEncoding EYRE_ENCODINGS_RDSEED[] = {
	{ 50959, 7, 0, 14 },
};

static EyreGroup eyreEncodings[] = {
	{ 4080, 5, EYRE_ENCODINGS_ADD },
	{ 4080, 5, EYRE_ENCODINGS_OR },
	{ 4080, 5, EYRE_ENCODINGS_ADC },
	{ 4080, 5, EYRE_ENCODINGS_SBB },
	{ 4080, 5, EYRE_ENCODINGS_AND },
	{ 4080, 5, EYRE_ENCODINGS_SUB },
	{ 4080, 5, EYRE_ENCODINGS_XOR },
	{ 4080, 5, EYRE_ENCODINGS_CMP },
	{ 49164, 3, EYRE_ENCODINGS_PUSH },
	{ 12, 3, EYRE_ENCODINGS_POP },
	{ 16384, 1, EYRE_ENCODINGS_MOVSXD },
	{ 49152, 1, EYRE_ENCODINGS_MOVSX },
	{ 49152, 1, EYRE_ENCODINGS_MOVZX },
	{ 1, 1, EYRE_ENCODINGS_INSB },
	{ 1, 1, EYRE_ENCODINGS_INSW },
	{ 1, 1, EYRE_ENCODINGS_INSD },
	{ 1, 1, EYRE_ENCODINGS_OUTSB },
	{ 1, 1, EYRE_ENCODINGS_OUTSW },
	{ 1, 1, EYRE_ENCODINGS_OUTSD },
	{ 49152, 1, EYRE_ENCODINGS_JA },
	{ 49152, 1, EYRE_ENCODINGS_JAE },
	{ 49152, 1, EYRE_ENCODINGS_JB },
	{ 49152, 1, EYRE_ENCODINGS_JBE },
	{ 49152, 1, EYRE_ENCODINGS_JC },
	{ 49152, 1, EYRE_ENCODINGS_JE },
	{ 49152, 1, EYRE_ENCODINGS_JG },
	{ 49152, 1, EYRE_ENCODINGS_JGE },
	{ 49152, 1, EYRE_ENCODINGS_JL },
	{ 49152, 1, EYRE_ENCODINGS_JLE },
	{ 49152, 1, EYRE_ENCODINGS_JNA },
	{ 49152, 1, EYRE_ENCODINGS_JNAE },
	{ 49152, 1, EYRE_ENCODINGS_JNB },
	{ 49152, 1, EYRE_ENCODINGS_JNBE },
	{ 49152, 1, EYRE_ENCODINGS_JNC },
	{ 49152, 1, EYRE_ENCODINGS_JNE },
	{ 49152, 1, EYRE_ENCODINGS_JNG },
	{ 49152, 1, EYRE_ENCODINGS_JNGE },
	{ 49152, 1, EYRE_ENCODINGS_JNL },
	{ 49152, 1, EYRE_ENCODINGS_JNLE },
	{ 49152, 1, EYRE_ENCODINGS_JNO },
	{ 49152, 1, EYRE_ENCODINGS_JNP },
	{ 49152, 1, EYRE_ENCODINGS_JNS },
	{ 49152, 1, EYRE_ENCODINGS_JNZ },
	{ 49152, 1, EYRE_ENCODINGS_JO },
	{ 49152, 1, EYRE_ENCODINGS_JP },
	{ 49152, 1, EYRE_ENCODINGS_JPE },
	{ 49152, 1, EYRE_ENCODINGS_JPO },
	{ 49152, 1, EYRE_ENCODINGS_JS },
	{ 49152, 1, EYRE_ENCODINGS_JZ },
	{ 2512, 1, EYRE_ENCODINGS_TEST },
	{ 49264, 1, EYRE_ENCODINGS_XCHG },
	{ 16880, 1, EYRE_ENCODINGS_MOV },
	{ 32, 1, EYRE_ENCODINGS_LEA },
	{ 7, 1, EYRE_ENCODINGS_NOP },
	{ 1, 1, EYRE_ENCODINGS_CBW },
	{ 1, 1, EYRE_ENCODINGS_CWDE },
	{ 1, 1, EYRE_ENCODINGS_CDQE },
	{ 1, 1, EYRE_ENCODINGS_CWD },
	{ 1, 1, EYRE_ENCODINGS_CDQ },
	{ 1, 1, EYRE_ENCODINGS_CQO },
	{ 1, 1, EYRE_ENCODINGS_WAIT },
	{ 1, 1, EYRE_ENCODINGS_FWAIT },
	{ 1, 1, EYRE_ENCODINGS_PUSHF },
	{ 1, 1, EYRE_ENCODINGS_PUSHFQ },
	{ 1, 1, EYRE_ENCODINGS_LAHF },
	{ 1, 1, EYRE_ENCODINGS_MOVSB },
	{ 1, 1, EYRE_ENCODINGS_MOVSW },
	{ 1, 1, EYRE_ENCODINGS_MOVSD },
	{ 1, 1, EYRE_ENCODINGS_MOVSQ },
	{ 1, 1, EYRE_ENCODINGS_CMPSB },
	{ 1, 1, EYRE_ENCODINGS_CMPSW },
	{ 1, 1, EYRE_ENCODINGS_CMPSD },
	{ 1, 1, EYRE_ENCODINGS_CMPSQ },
	{ 1, 1, EYRE_ENCODINGS_STOSB },
	{ 1, 1, EYRE_ENCODINGS_STOSW },
	{ 1, 1, EYRE_ENCODINGS_STOSD },
	{ 1, 1, EYRE_ENCODINGS_STOSQ },
	{ 1, 1, EYRE_ENCODINGS_SCASB },
	{ 1, 1, EYRE_ENCODINGS_SCASW },
	{ 1, 1, EYRE_ENCODINGS_SCASD },
	{ 1, 1, EYRE_ENCODINGS_SCASQ },
	{ 1, 1, EYRE_ENCODINGS_LODSB },
	{ 1, 1, EYRE_ENCODINGS_LODSW },
	{ 1, 1, EYRE_ENCODINGS_LODSD },
	{ 1, 1, EYRE_ENCODINGS_LODSQ },
	{ 13824, 28, EYRE_ENCODINGS_ROL },
	{ 13824, 28, EYRE_ENCODINGS_ROR },
	{ 13824, 28, EYRE_ENCODINGS_RCL },
	{ 13824, 28, EYRE_ENCODINGS_RCR },
	{ 13824, 28, EYRE_ENCODINGS_SAL },
	{ 13824, 28, EYRE_ENCODINGS_SHL },
	{ 13824, 28, EYRE_ENCODINGS_SHR },
	{ 13824, 28, EYRE_ENCODINGS_SAR },
	{ 16385, 1, EYRE_ENCODINGS_RET },
	{ 16385, 1, EYRE_ENCODINGS_RETF },
	{ 1, 1, EYRE_ENCODINGS_LEAVEW },
	{ 1, 1, EYRE_ENCODINGS_LEAVE },
	{ 1, 1, EYRE_ENCODINGS_INT3 },
	{ 16384, 1, EYRE_ENCODINGS_INT },
	{ 1, 1, EYRE_ENCODINGS_INT1 },
	{ 1, 1, EYRE_ENCODINGS_IRETW },
	{ 1, 1, EYRE_ENCODINGS_IRETD },
	{ 1, 1, EYRE_ENCODINGS_IRETQ },
	{ 16384, 1, EYRE_ENCODINGS_JCXZ },
	{ 16384, 1, EYRE_ENCODINGS_JRCXZ },
	{ 49152, 1, EYRE_ENCODINGS_IN },
	{ 49152, 1, EYRE_ENCODINGS_OUT },
	{ 1, 1, EYRE_ENCODINGS_HLT },
	{ 1, 1, EYRE_ENCODINGS_CMC },
	{ 6, 1, EYRE_ENCODINGS_NOT },
	{ 6, 1, EYRE_ENCODINGS_NEG },
	{ 6, 1, EYRE_ENCODINGS_MUL },
	{ 49206, 1, EYRE_ENCODINGS_IMUL },
	{ 6, 1, EYRE_ENCODINGS_DIV },
	{ 6, 1, EYRE_ENCODINGS_IDIV },
	{ 1, 1, EYRE_ENCODINGS_CLC },
	{ 1, 1, EYRE_ENCODINGS_STC },
	{ 1, 1, EYRE_ENCODINGS_CLI },
	{ 1, 1, EYRE_ENCODINGS_STI },
	{ 1, 1, EYRE_ENCODINGS_CLD },
	{ 1, 1, EYRE_ENCODINGS_STD },
	{ 6, 1, EYRE_ENCODINGS_INC },
	{ 6, 1, EYRE_ENCODINGS_DEC },
	{ 16390, 1, EYRE_ENCODINGS_CALL },
	{ 4, 1, EYRE_ENCODINGS_CALLF },
	{ 49158, 1, EYRE_ENCODINGS_JMP },
	{ 4, 1, EYRE_ENCODINGS_JMPF },
	{ 2, 1, EYRE_ENCODINGS_RDRAND },
	{ 2, 1, EYRE_ENCODINGS_RDSEED },
};

EyreGroup* eyreGetEncodings(int mnemonic) { return &eyreEncodings[mnemonic]; }


