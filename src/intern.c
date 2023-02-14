#include "intern.h"
#include <mem.h>
#include "enums.h"
#include "mnemonics.h"



typedef struct {
	int intern;
	int next;
} InternNode;



static InternNode* nodes     = NULL;
static int nodesCapacity     = 512;
static int nodesSize         = 1;

static StringIntern* strings  = NULL;
static int stringsCapacity    = 8192;
static int stringsSize        = 1;
static int stringsBucketsSize = 8192;
static int stringsBuckets     [8192];

static ScopeIntern* scopes    = NULL;
static int scopesCapacity     = 1024;
static int scopesSize         = 1;
static int scopesBucketsSize  = 1024;
static int scopesBuckets      [1024];

static SymBase** symbols      = NULL;
static int symbolsCapacity    = 4096;
static int symbolsSize        = 1;
static int symbolsBucketsSize = 4096;
static int symbolsBuckets     [4096];



static int addNode(int intern, int next) {
	checkCapacity((void**) &nodes, nodesSize, &nodesCapacity, sizeof(InternNode));
	InternNode* node = &nodes[nodesSize++];
	node->intern = intern;
	node->next = next;
	return nodesSize - 1;
}



static int addString(char* data, int length, int hash) {
	void* newData = palloc(length + 1);
	memcpy(newData, data, length);
	checkCapacity((void**) &strings, stringsSize, &stringsCapacity, sizeof(StringIntern));
	StringIntern* string = &strings[stringsSize++];
	string->hash   = hash;
	string->length = length;
	string->data   = newData;
	return stringsSize - 1;
}

static int addScope(int* data, int length, int hash) {
	void* newData = palloc(length * sizeof(int));
	memcpy(newData, data, length * sizeof(int));
	checkCapacity((void**) &scopes, scopesSize, &scopesCapacity, sizeof(ScopeIntern));
	ScopeIntern* scope = &scopes[scopesSize++];
	scope->hash   = hash;
	scope->length = length;
	scope->data   = newData;
	return scopesSize - 1;
}

static int addSymbol(SymBase* symbol) {
	checkCapacity((void**) &symbols, symbolsSize, &symbolsCapacity, sizeof(void*));
	symbols[symbolsSize++] = symbol;
	return symbolsSize - 1;
}



static int matchString(char* data, int length, int hash, int index) {
	StringIntern* string = &strings[index];
	return hash == string->hash && length == string->length && (memcmp(data, string->data, length * sizeof(char)) == 0);
}

static int matchScope(int* data, int length, int hash, int index) {
	ScopeIntern* scope = &scopes[index];
	return hash == scope->hash && length == scope->length && (memcmp(data, scope->data, length * sizeof(int)) == 0);
}

static int matchSymbol(int scope, int name, int index) {
	SymBase* symbol = symbols[index];
	return scope == symbol->scope && name == symbol->name;
}



int eyreInternString(char* data, int length) {
	int hash = 0;
	for(int i = 0; i < length; i++)
		hash = 31 * hash + (data[i]);

	int bucketIndex = (unsigned int) hash % stringsBucketsSize;
	int* buckets = stringsBuckets;
	int bucket = buckets[bucketIndex];

	if(bucket == 0) {
		int id = addString(data, length, hash);
		buckets[bucketIndex] = id << 1;
		return id;
	}

	if((bucket & 1) == 0) {
		int internIndex = bucket >> 1;
		if(matchString(data, length, hash, internIndex))
			return internIndex;
		int id = addString(data, length, hash);
		addNode(bucket >> 1, nodesSize + 1);
		addNode(id, 0);
		buckets[bucketIndex] = ((nodesSize - 2) << 1) | 1;
		return id;
	}

	InternNode* node = &nodes[bucket >> 1];

	while(1) {
		if(matchString(data, length, hash, node->intern))
			return node->intern;
		if(node->next == 0) {
			int id = addString(data, length, hash);
			node->next = nodesSize;
			addNode(id, 0);
			return id;
		}
		node = &nodes[node->next];
	}
}



int eyreInternScope(int* data, int length, int hash) {
	int bucketIndex = (unsigned int) hash % scopesBucketsSize;
	int* buckets = scopesBuckets;
	int bucket = buckets[bucketIndex];

	if(bucket == 0) {
		int id = addScope(data, length, hash);
		buckets[bucketIndex] = id << 1;
		return id;
	}

	if((bucket & 1) == 0) {
		int internIndex = bucket >> 1;
		if(matchScope(data, length, hash, internIndex))
			return internIndex;
		int id = addScope(data, length, hash);
		addNode(bucket >> 1, nodesSize + 1);
		addNode(id, 0);
		buckets[bucketIndex] = ((nodesSize - 2) << 1) | 1;
		return id;
	}

	InternNode* node = &nodes[bucket >> 1];

	while(1) {
		if(matchScope(data, length, hash, node->intern))
			return node->intern;
		if(node->next == 0) {
			int id = addScope(data, length, hash);
			node->next = nodesSize;
			addNode(id, 0);
			return id;
		}
		node = &nodes[node->next];
	}
}



int eyreInternSymbol(SymBase* symbol) {
	int scope = symbol->scope;
	int name = symbol->name;
	int hash = scope * 31 + name;
	int bucketIndex = (unsigned int) hash % symbolsBucketsSize;
	int* buckets = symbolsBuckets;
	int bucket = buckets[bucketIndex];

	if(bucket == 0) {
		int id = addSymbol(symbol);
		buckets[bucketIndex] = id << 1;
		return id;
	}

	if((bucket & 1) == 0) {
		int internIndex = bucket >> 1;
		if(matchSymbol(scope, name, internIndex))
			return internIndex;
		int id = addSymbol(symbol);
		addNode(bucket >> 1, nodesSize + 1);
		addNode(id, 0);
		buckets[bucketIndex] = ((nodesSize - 2) << 1) | 1;
		return id;
	}

	InternNode* node = &nodes[bucket >> 1];

	while(1) {
		if(matchSymbol(scope, name, node->intern))
			return node->intern;
		if(node->next == 0) {
			int id = addSymbol(symbol);
			node->next = nodesSize;
			addNode(id, 0);
			return id;
		}
		node = &nodes[node->next];
	}
}



void* eyreResolveSymbol(int scope, int name) {
	int hash = scope * 31 + name;
	int bucketIndex = (unsigned int) hash % symbolsBucketsSize;
	int* buckets = symbolsBuckets;
	int bucket = buckets[bucketIndex];

	if(bucket == 0) return NULL;

	if((bucket & 1) == 0) {
		int internIndex = bucket >> 1;
		if(matchSymbol(scope, name, internIndex))
			return symbols[internIndex];
		return NULL;
	}

	InternNode* node = &nodes[bucket >> 1];

	while(1) {
		if(matchSymbol(scope, name, node->intern))
			return symbols[node->intern];
		if(node->next == 0)
			return NULL;
		node = &nodes[node->next];
	}
}



void* eyreAddSymbol(char type, int scope, int name, int size) {
	SymBase* symbol = palloc(size);
	symbol->type    = type;
	symbol->scope   = scope;
	symbol->name    = name;
	eyreInternSymbol(symbol);
	return symbol;
}



static ScopeIntern globalScope = { .data = NULL, .length = 0, .hash = 0 };



StringIntern* eyreGetString(int id) {
	return &strings[id];
}

ScopeIntern* eyreGetScope(int id) {
	if(id == 0) return &globalScope;
	return &scopes[id];
}

SymBase* eyreGetSymbol(int id) {
	return symbols[id];
}



// Printing



static void printSymbol(SymBase* symbol) {
	ScopeIntern* scope = eyreGetScope(symbol->scope);
	for(int i = 0; i < scope->length; i++) {
		printf("%s.", eyreGetString(scope->data[i])->data);
	}
	printf("%s\n", eyreGetString(symbol->name)->data);
}



void eyrePrintSymbols() {
	for(int i = 0; i < symbolsBucketsSize; i++) {
		int bucket = symbolsBuckets[i];

		if(bucket == 0)
			continue;

		if((bucket & 1) == 0) {
			printSymbol(symbols[bucket >> 1]);
			continue;
		}

		InternNode node;

		do {
			node = nodes[bucket >> 1];
			printSymbol(symbols[node.intern]);
		} while(node.next != 0);
	}
}




// Standard interns



static int addStandardIntern(char* string) {
	return eyreInternString(string, (int) strlen(string));
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



int EYRE_INTERN_MAIN;



void eyreInitInterns() {
	// Keywords
	keywordStart = stringsSize;
	for(int i = 0; i < KEYWORD_COUNT; i++)
		addStandardIntern(eyreKeywordNames[i]);
	keywordCount = stringsSize - keywordStart;

	// Widths
	widthStart = stringsSize;
	for(int i = 0; i < WIDTH_COUNT; i++)
		addStandardIntern(eyreWidthNames[i]);
	widthCount = stringsSize - widthStart;

	// General-purpose registers
	registerStart = stringsSize;
	for(int i = 0; i < 16; i++)
		addStandardIntern(eyreByteRegNames[i]);
	for(int i = 0; i < 16; i++)
		addStandardIntern(eyreWordRegNames[i]);
	for(int i = 0; i < 16; i++)
		addStandardIntern(eyreDWordRegNames[i]);
	for(int i = 0; i < 16; i++)
		addStandardIntern(eyreQWordRegNames[i]);
	registerCount = stringsSize - registerStart;

	// Mnemonics
	mnemonicStart = stringsSize;
	for(int i = 0; i < MNEMONIC_COUNT; i++)
		addStandardIntern(eyreMnemonicNames[i]);
	mnemonicCount = stringsSize - mnemonicStart;

	EYRE_INTERN_MAIN = addStandardIntern("main");
}