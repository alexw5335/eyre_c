#include "eyre.h"
#include <mem.h>



typedef struct {
	int intern;
	int next;
} InternNode;



static List eyreInternList = { .size = 1, .capacity = 1024 };

static List nodeList = { .size = 1, .capacity = 1024 };

const int bucketCount = 16384;

static int buckets[16384];



static int stringHash(const char* string, int length) {
	int hash = 0;
	for(int i = 0; i < length; i++)
		hash = 31 * hash + (string[i]);
	return hash;
}



Intern* eyreGetIntern(u32 id) {
	Intern* interns = eyreInternList.data;
	return &interns[id];
}



static int addInternToList(char* string, int length, int hash, int copy) {
	eyreCheckListCapacity(&eyreInternList, sizeof(Intern));

	if(copy) {
		char* newString = eyreAlloc(length + 1);
		memcpy(newString, string, length);
		string = newString;
	}


	int id = eyreInternList.size;
	Intern* interns = eyreInternList.data;
	Intern* intern = &interns[eyreInternList.size++];
	intern->id = id;
	intern->length = length;
	intern->hash = hash;
	intern->flags = copy;
	intern->string = string;
	return id;
}



int eyreAddIntern(char* string, int length, int copy) {
	int hash = stringHash(string, length);
	int bucketIndex = (int) ((u32) hash % bucketCount);
	int bucket = buckets[bucketIndex];

	// Empty bucket, simply add intern and return
	if(bucket == 0) {
		int internId = addInternToList(string, length, hash, copy);
		buckets[bucketIndex] = internId << 1;
		return internId;
	}

	eyreCheckListCapacity(&nodeList, sizeof(InternNode));
	Intern* interns = eyreInternList.data;
	InternNode* nodes = nodeList.data;

	// Bucket contains a single intern
	if((bucket & 1) == 0) {
		Intern intern = interns[bucket >> 1];

		// If the existing intern matches the input, return it
		if(hash == intern.hash && length == intern.length && memcmp(string, intern.string, length) == 0)
			return intern.id;

		// Otherwise, the bucket is changed to refer to a linked list node
		int internId = addInternToList(string, length, hash, copy);

		eyreCheckListCapacity(&nodeList, sizeof(InternNode));
		nodes[nodeList.size].intern = bucket >> 1;
		nodes[nodeList.size].next = nodeList.size + 1;
		nodeList.size++;

		eyreCheckListCapacity(&nodeList, sizeof(InternNode));
		nodes[nodeList.size].intern = internId;
		nodes[nodeList.size].next = 0;
		nodeList.size++;

		buckets[bucketIndex] = ((nodeList.size - 2) << 1) | 1;

		return internId;
	}

	// Bucket refers to a linked list of interns
	Intern intern;
	InternNode* node = &nodes[bucket >> 1];
	while(1) {
		intern = interns[node->intern];

		// Existing intern found
		if(hash == intern.hash && length == intern.length && memcmp(string, intern.string, length) == 0)
			return intern.id;

		// If the intern does not exist, then append the new intern to the end of the linked list
		if(node->next == 0) {
			int internId = addInternToList(string, length, hash, copy);
			node->next = nodeList.size;
			eyreCheckListCapacity(&nodeList, sizeof(InternNode));
			nodes[nodeList.size].intern = internId;
			nodes[nodeList.size].next = 0;
			nodeList.size++;
			return internId;
		}

		node = &nodes[node->next];
	}
}



// Standard interns



int eyreRegisterInternStart;
int eyreRegisterInternCount;
int eyreRegisterInternEnd;

int eyreKeywordInternStart;
int eyreKeywordInternCount;
int eyreKeywordInternEnd;

int eyreWidthInternStart;
int eyreWidthInternCount;
int eyreWidthInternEnd;

int eyreMnemonicInternStart;
int eyreMnemonicInternCount;
int eyreMnemonicInternEnd;



static int addStandardIntern(char* string) {
	return eyreAddIntern(string, (int) strlen(string), FALSE);
}



void eyreInitInterns() {
	// Keywords
	eyreKeywordInternStart = eyreInternList.size;
	for(int i = 0; i < KEYWORD_COUNT; i++)
		addStandardIntern(eyreKeywordNames[i]);
	eyreKeywordInternEnd = eyreInternList.size;
	eyreKeywordInternCount = eyreKeywordInternEnd - eyreKeywordInternStart;

	// Widths
	eyreWidthInternStart = eyreInternList.size;
	for(int i = 0; i < WIDTH_COUNT; i++)
		addStandardIntern(eyreWidthNames[i]);
	eyreWidthInternEnd = eyreInternList.size;
	eyreWidthInternCount = eyreWidthInternEnd - eyreWidthInternStart;

	// General-purpose registers
	eyreRegisterInternStart = eyreInternList.size;
	for(int i = 0; i < 16; i++)
		addStandardIntern(eyreByteRegNames[i]);
	for(int i = 0; i < 16; i++)
		addStandardIntern(eyreWordRegNames[i]);
	for(int i = 0; i < 16; i++)
		addStandardIntern(eyreDWordRegNames[i]);
	for(int i = 0; i < 16; i++)
		addStandardIntern(eyreQWordRegNames[i]);
	eyreRegisterInternEnd = eyreInternList.size;
	eyreRegisterInternCount = eyreRegisterInternEnd - eyreRegisterInternStart;

	// Mnemonics
	eyreMnemonicInternStart = eyreInternList.size;
	for(int i = 0; i < MNEMONIC_COUNT; i++)
		addStandardIntern(eyreMnemonicNames[i]);
	eyreMnemonicInternEnd = eyreInternList.size;
	eyreMnemonicInternCount = eyreMnemonicInternEnd - eyreMnemonicInternStart;
}