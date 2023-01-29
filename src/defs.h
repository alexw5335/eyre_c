#ifndef EYRE_DEFS_INCLUDE
#define EYRE_DEFS_INCLUDE



// Primitives



#define TRUE 1
#define FALSE 0

typedef unsigned char      u8;
typedef signed char        s8;
typedef unsigned short     u16;
typedef signed short       s16;
typedef unsigned int       u32;
typedef signed int         s32;
typedef unsigned long long u64;
typedef signed long long   s64;



// Enums



typedef enum {
	NODE_REG,
	NODE_INT,
	NODE_SYM,
	NODE_BINARY,
	NODE_UNARY,
	NODE_MEM,
	NODE_IMM,
	NODE_INS,
	NODE_STRUCT,
	NODE_SCOPE_END,
	NODE_NAMESPACE,
	NODE_ENUM,
	NODE_LABEL,
	NODE_DOT,
	NODE_INVOKE,
	NODE_COUNT
} EyreNodeType;



typedef enum EyreUnaryOp {
	UNARY_POS,
	UNARY_NEG,
	UNARY_NOT,
	UNARY_COUNT
} EyreUnaryOp;



typedef enum EyreBinaryOp {
	BINARY_ADD,
	BINARY_SUB,
	BINARY_MUL,
	BINARY_DIV,
	BINARY_AND,
	BINARY_OR,
	BINARY_XOR,
	BINARY_SHL,
	BINARY_SHR,
	BINARY_DOT,
	BINARY_INV,
	BINARY_COUNT
} EyreBinaryOp;



typedef enum RegisterType {
	REGISTER_BYTE,
	REGISTER_WORD,
	REGISTER_DWORD,
	REGISTER_QWORD,
	REGISTER_COUNT
} RegisterType;



typedef enum {
	KEYWORD_CONST,
	KEYWORD_STRUCT,
	KEYWORD_NAMESPACE,
	KEYWORD_PROC,
	KEYWORD_ENUM,
	KEYWORD_BITMASK,
	KEYWORD_COUNT,
} EyreKeyword;



typedef enum {
	WIDTH_BYTE,
	WIDTH_WORD,
	WIDTH_DWORD,
	WIDTH_QWORD,
	WIDTH_COUNT,
} EyreWidth;



typedef enum EyreMnemonic {
	MNEMONIC_ADD8, MNEMONIC_OR8, MNEMONIC_ADC8, MNEMONIC_SBB8,
	MNEMONIC_AND8, MNEMONIC_SUB8, MNEMONIC_XOR8, MNEMONIC_CMP8,
	MNEMONIC_JMP8, MNEMONIC_ADD, MNEMONIC_OR, MNEMONIC_ADC,
	MNEMONIC_SBB, MNEMONIC_AND, MNEMONIC_SUB, MNEMONIC_XOR,
	MNEMONIC_CMP, MNEMONIC_PUSH, MNEMONIC_PUSH_FS, MNEMONIC_PUSH_GS,
	MNEMONIC_PUSHW_FS, MNEMONIC_PUSHW_GS, MNEMONIC_POP, MNEMONIC_POP_FS,
	MNEMONIC_POP_GS, MNEMONIC_POPW_FS, MNEMONIC_POPW_GS, MNEMONIC_MOVSXD,
	MNEMONIC_MOVSX, MNEMONIC_MOVZX, MNEMONIC_INSB, MNEMONIC_INSW,
	MNEMONIC_INSD, MNEMONIC_OUTSB, MNEMONIC_OUTSW, MNEMONIC_OUTSD,
	MNEMONIC_JA, MNEMONIC_JAE, MNEMONIC_JB, MNEMONIC_JBE,
	MNEMONIC_JC, MNEMONIC_JE, MNEMONIC_JG, MNEMONIC_JGE,
	MNEMONIC_JL, MNEMONIC_JLE, MNEMONIC_JNA, MNEMONIC_JNAE,
	MNEMONIC_JNB, MNEMONIC_JNBE, MNEMONIC_JNC, MNEMONIC_JNE,
	MNEMONIC_JNG, MNEMONIC_JNGE, MNEMONIC_JNL, MNEMONIC_JNLE,
	MNEMONIC_JNO, MNEMONIC_JNP, MNEMONIC_JNS, MNEMONIC_JNZ,
	MNEMONIC_JO, MNEMONIC_JP, MNEMONIC_JPE, MNEMONIC_JPO,
	MNEMONIC_JS, MNEMONIC_JZ, MNEMONIC_TEST, MNEMONIC_XCHG,
	MNEMONIC_MOV, MNEMONIC_LEA, MNEMONIC_NOP, MNEMONIC_CBW,
	MNEMONIC_CWDE, MNEMONIC_CDQE, MNEMONIC_CWD, MNEMONIC_CDQ,
	MNEMONIC_CQO, MNEMONIC_WAIT, MNEMONIC_FWAIT, MNEMONIC_PUSHF,
	MNEMONIC_PUSHFQ, MNEMONIC_LAHF, MNEMONIC_MOVSB, MNEMONIC_MOVSW,
	MNEMONIC_MOVSD, MNEMONIC_MOVSQ, MNEMONIC_CMPSB, MNEMONIC_CMPSW,
	MNEMONIC_CMPSD, MNEMONIC_CMPSQ, MNEMONIC_STOSB, MNEMONIC_STOSW,
	MNEMONIC_STOSD, MNEMONIC_STOSQ, MNEMONIC_SCASB, MNEMONIC_SCASW,
	MNEMONIC_SCASD, MNEMONIC_SCASQ, MNEMONIC_LODSB, MNEMONIC_LODSW,
	MNEMONIC_LODSD, MNEMONIC_LODSQ, MNEMONIC_ROL, MNEMONIC_ROR,
	MNEMONIC_RCL, MNEMONIC_RCR, MNEMONIC_SAL, MNEMONIC_SHL,
	MNEMONIC_SHR, MNEMONIC_SAR, MNEMONIC_RET, MNEMONIC_RETF,
	MNEMONIC_LEAVEW, MNEMONIC_LEAVE, MNEMONIC_INT3, MNEMONIC_INT,
	MNEMONIC_INT1, MNEMONIC_IRETW, MNEMONIC_IRETD, MNEMONIC_IRETQ,
	MNEMONIC_JCXZ, MNEMONIC_JRCXZ, MNEMONIC_IN, MNEMONIC_OUT,
	MNEMONIC_HLT, MNEMONIC_CMC, MNEMONIC_NOT, MNEMONIC_NEG,
	MNEMONIC_MUL, MNEMONIC_IMUL, MNEMONIC_DIV, MNEMONIC_IDIV,
	MNEMONIC_CLC, MNEMONIC_STC, MNEMONIC_CLI, MNEMONIC_STI,
	MNEMONIC_CLD, MNEMONIC_STD, MNEMONIC_INC, MNEMONIC_DEC,
	MNEMONIC_CALL, MNEMONIC_CALLF, MNEMONIC_JMP, MNEMONIC_JMPF,
	MNEMONIC_RDRAND, MNEMONIC_RDSEED,
	MNEMONIC_COUNT
} EyreMnemonic;



typedef enum {
	SPECIFIER_NONE,
	SPECIFIER_O,
	SPECIFIER_I8,
	SPECIFIER_I16,
	SPECIFIER_I32,
	SPECIFIER_REL8,
	SPECIFIER_REL32,
	SPECIFIER_RM_I8,
	SPECIFIER_RM_1,
	SPECIFIER_RM_CL,
	SPECIFIER_COUNT,
} EyreSpecifier;



typedef enum {
	OPERANDS_NONE,
	OPERANDS_R,
	OPERANDS_M,
	OPERANDS_O,
	OPERANDS_I8,
	OPERANDS_I16,
	OPERANDS_I32,
	OPERANDS_REL8,
	OPERANDS_REL32,
	OPERANDS_R_R,
	OPERANDS_R_M,
	OPERANDS_M_R,
	OPERANDS_R_I,
	OPERANDS_M_I,
	OPERANDS_R_I8,
	OPERANDS_M_I8,
	OPERANDS_A_I,
	OPERANDS_RM_1,
	OPERANDS_RM_CL,
	OPERANDS_CUSTOM1,
	OPERANDS_CUSTOM2,
	OPERANDS_COUNT,
} EyreOperands;



typedef enum {
	COMPOUND_RM,
	COMPOUND_RM_R,
	COMPOUND_R_RM,
	COMPOUND_RM_I,
	COMPOUND_RM_I8,
	COMPOUND_COUNT,
} EyreCompoundOperands;



extern char* eyreWidthNames[WIDTH_COUNT];

extern char* eyreOperandsNames[OPERANDS_COUNT];

extern int eyreOperandsSpecifiers[OPERANDS_COUNT];

extern char* eyreCompoundOperandsNames[COMPOUND_COUNT];

extern EyreOperands eyreCompoundOperandsMap[COMPOUND_COUNT][3];

extern char* eyreNodeNames[NODE_COUNT];

extern char* eyreUnaryOpSymbols[UNARY_COUNT];

extern char* eyreBinaryOpSymbols[BINARY_COUNT];

extern char* eyreByteRegNames[16];

extern char* eyreWordRegNames[16];

extern char* eyreDWordRegNames[16];

extern char* eyreQWordRegNames[16];

extern char* eyreKeywordNames[KEYWORD_COUNT];

extern char* eyreMnemonicNames[MNEMONIC_COUNT];

extern char* eyreSpecifierNames[SPECIFIER_COUNT];



// ENCODINGS



typedef struct {
	int opcode;
	char extension;
	char prefix;
	char widths;
} EyreEncoding;



typedef struct {
	int operandsBits;
	int specifierBits;
	EyreEncoding* encodings;
} EyreGroup;



typedef struct {
	char* mnemonic;
	int opcode;
	int extension;
	EyreOperands operands;
	int widths;
} EyreGenEncoding;



typedef struct {
	char mnemonic[16];
	int operandsBits;
	int specifierBits;
	int encodingCount;
	EyreGenEncoding* encodings[32];
} EyreGenGroup;



// MISC



typedef struct SrcFile {
	char*  path;
	char*  chars;
	int    size;
	void** nodes;
	short* nodeLines;
	int    nodeCount;
} SrcFile;



typedef struct {
	void* data;
	int size;
	int capacity;
} List;



typedef struct {
	List  banks;
	void* start;
	void* current;
	void* end;
	int   bankCapacity;
} Buffer;



typedef struct {
	int hash;
	int length;
	char* data;
} StringIntern;



typedef struct {
	int hash;
	int length;
	int* data;
} ScopeIntern;



typedef struct {
	int   pos;
	int   disp;
	void* symbol;
} Relocation;



#endif