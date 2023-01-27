#include "eyre.h"
#include "intern.h"
#include <mem.h>



typedef struct {
	int intern;
	int next;
} InternNode;


static List arrayList = { .size = 1, .capacity = 512 };

static List internList = { .size = 1, .capacity = 1024 };

static List nodeList = { .size = 1, .capacity = 1024 };

static const int bucketCount = 16384;

static int buckets[16384];

typedef void (*Inserter)(void* data, int length, int hash, int copy);
typedef int (*Matcher)(void* data, int length, int hash, int index);

typedef struct {
	List*    list;
	int*     buckets;
	int      bucketCount;
	int      elementSize;
	Inserter inserter;
	Matcher  matcher;
} Interner;

static List stringList = { .size = 1, .capacity = 4096 };



static void insertString(void* data, int length, int hash, int copy) {
	Intern* interns = internList.data;
	Intern* intern  = &interns[internList.size++];
	intern->length  = length;
	intern->hash    = hash;
	intern->string  = data;
}



static int stringBuckets[8192];

static int arrayBuckets[8192];



static Interner stringInterner = {
	.buckets = stringBuckets,
	.bucketCount = 8192,
	.elementSize = sizeof(Intern),
	.inserter = insertString,
};




static int addIntern(Interner* interner, void* data, int length, int hash, int copy) {
	eyreCheckListCapacity(interner->list, interner->elementSize);

	if(copy) {
		char* newString = eyreAllocPersistent(length + 1);
		memcpy(newString, data, length);
		data = newString;
	}

	int id = interner->list->size;
	interner->inserter(data, length, hash, copy);
	return id;
}



static int intern(Interner* interner, void* data, int length, int hash, int copy) {
	int bucketIndex = hash & interner->bucketCount;
	int bucket = interner->buckets[bucketIndex];

	// Empty bucket, simply add intern and return
	if(bucket == 0) {
		int internId = addIntern(interner, data, length, hash, copy);
		interner->buckets[bucketIndex] = internId << 1;
		return internId;
	}

	eyreCheckListCapacity(&nodeList, sizeof(InternNode));
	InternNode* nodes = nodeList.data;

	// Bucket contains a single intern
	if((bucket & 1) == 0) {
		int internIndex = bucket >> 1;

		// If the existing intern matches the input, return it
		if(interner->matcher(data, length, hash, internIndex)) return internIndex;

		// Otherwise, the bucket is changed to refer to a linked list node
		int internId = addIntern(interner, data, length, hash, copy);

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
	InternNode* node = &nodes[bucket >> 1];
	while(1) {
		// Existing intern found
		if(interner->matcher(data, length, hash, node->intern))
			return node->intern;

		// If the intern does not exist, then append the new intern to the end of the linked list
		if(node->next == 0) {
			int internId = addIntern(interner, data, length, hash, copy);
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



static int stringHash(const char* string, int length) {
	int hash = 0;
	for(int i = 0; i < length; i++)
		hash = 31 * hash + (string[i]);
	return hash;
}



Intern* eyreGetIntern(u32 id) {
	Intern* interns = internList.data;
	return &interns[id];
}



static int addInternToList(char* string, int length, int hash, int copy) {
	eyreCheckListCapacity(&internList, sizeof(Intern));

	if(copy) {
		char* newString = eyreAllocPersistent(length + 1);
		memcpy(newString, string, length);
		string = newString;
	}


	int id = internList.size;
	Intern* interns = internList.data;
	Intern* intern = &interns[internList.size++];
	intern->length = length;
	intern->hash = hash;
	intern->string = string;
	return id;
}



static int addArrayToList(int* components, int length, int hash, int copy) {
	eyreCheckListCapacity(&arrayList, sizeof(InternArray));

	if(copy) {
		int* newComponents = eyreAllocPersistent(length * sizeof(int));
		memcpy(newComponents, components, length * sizeof(int));
		components = newComponents;
	}

	int id = arrayList.size;
	InternArray* arrays = arrayList.data;
	InternArray* array = &arrays[arrayList.size++];
	array->length = length;
	array->hash = hash;
	array->components = components;
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
	Intern* interns = internList.data;
	InternNode* nodes = nodeList.data;

	// Bucket contains a single intern
	if((bucket & 1) == 0) {
		int internIndex = bucket >> 1;
		Intern intern = interns[internIndex];

		// If the existing intern matches the input, return it
		if(hash == intern.hash && length == intern.length && memcmp(string, intern.string, length) == 0)
			return internIndex;

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
			return node->intern;

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



static int addStandardIntern(char* string) {
	return eyreAddIntern(string, (int) strlen(string), FALSE);
}

static int registerStart;
static int registerCount;

static int keywordStart;
static int keywordCount;

static int widthStart;
static int widthCount;

static int mnemonicStart;
static int mnemonicCount;

int eyreInternToRegister(int intern) {
	int value = intern - registerStart;
	return value < registerCount ? value : -1;
}

int eyreInternToKeyword(int intern) {
	int value = intern - keywordStart;
	return value < keywordCount ? value : -1;
}

int eyreInternToWidth(int intern) {
	int value = intern - widthStart;
	return value < widthCount ? value : -1;
}

int eyreInternToMnemonic(int intern) {
	int value = intern - mnemonicStart;
	return value < mnemonicCount ? value : -1;
}



void eyreInitInterns() {
	// Keywords
	keywordStart = internList.size;
	for(int i = 0; i < KEYWORD_COUNT; i++)
		addStandardIntern(eyreKeywordNames[i]);
	keywordCount = internList.size - keywordStart;

	// Widths
	widthStart = internList.size;
	for(int i = 0; i < WIDTH_COUNT; i++)
		addStandardIntern(eyreWidthNames[i]);
	widthCount = internList.size - widthStart;

	// General-purpose registers
	registerStart = internList.size;
	for(int i = 0; i < 16; i++)
		addStandardIntern(eyreByteRegNames[i]);
	for(int i = 0; i < 16; i++)
		addStandardIntern(eyreWordRegNames[i]);
	for(int i = 0; i < 16; i++)
		addStandardIntern(eyreDWordRegNames[i]);
	for(int i = 0; i < 16; i++)
		addStandardIntern(eyreQWordRegNames[i]);
	registerCount = internList.size - registerStart;

	// Mnemonics
	mnemonicStart = internList.size;
	for(int i = 0; i < MNEMONIC_COUNT; i++)
		addStandardIntern(eyreMnemonicNames[i]);
	mnemonicCount = internList.size - mnemonicStart;
}