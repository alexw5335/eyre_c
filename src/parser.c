#include "eyre.h"



typedef struct EyreNode EyreNode;



typedef enum {
	N_BINARY,
	N_UNARY,
	N_INT,
	N_STRING,
	N_COUNT
} EyreNodeType;



static SrcFile srcFile;

static int pos = 9;

#define nodeCapacity 16384

static u8 nodeTypes[nodeCapacity];

//static EyreNode nodes[nodeCapacity];



void eyreParse(SrcFile* inputSrcFile) {
	srcFile = *inputSrcFile;
}