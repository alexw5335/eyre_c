#include "eyre.h"
#include <mem.h>



static List internList = { .size = 1, .capacity = 1024 };

static List nodeList = { .size = 1, .capacity = 1024 };

const int bucketCount = 16384;

static int buckets[16384];



static int stringHash(const char* string, int length) {
	int hash = 0;
	for(int i = 0; i < length; i++)
		hash = 31 * hash + (string[i]);
	return hash;
}



static int addInternToList(char* string, int length, int hash) {
	listEnsureCapacity(&internList, sizeof(Intern));

	int id = internList.size;
	Intern* interns = internList.data;
	Intern* intern = &interns[internList.size++];
	intern->id = id;
	intern->length = length;
	intern->hash = hash;
	intern->flags = 0;
	intern->string = string;
	return id;
}



int eyreIntern(char* string, int length) {
	int hash = stringHash(string, length);
	int bucketIndex = hash % bucketCount;
	int bucket = buckets[bucketIndex];

	// Empty bucket, simply add intern and return
	if(bucket == 0) {
		int internId = addInternToList(string, length, hash);
		buckets[bucketIndex] = internId << 1;
		return internId;
	}

	listEnsureCapacity(&nodeList, sizeof(Node));
	Intern* interns = internList.data;
	Node* nodes = nodeList.data;

	// Bucket contains a single intern
	if((bucket & 1) == 0) {
		Intern intern = interns[bucket >> 1];

		// If the existing intern matches the input, return it
		if(hash == intern.hash && length == intern.length && memcmp(string, intern.string, length) == 0)
			return intern.id;

		// Otherwise, the bucket is changed to refer to a linked list node
		int internId = addInternToList(string, length, hash);

		listEnsureCapacity(&nodeList, sizeof(Node));
		nodes[nodeList.size].intern = bucket >> 1;
		nodes[nodeList.size].next = nodeList.size + 1;
		nodeList.size++;

		listEnsureCapacity(&nodeList, sizeof(Node));
		nodes[nodeList.size].intern = internId;
		nodes[nodeList.size].next = 0;
		nodeList.size++;

		buckets[bucketIndex] = ((nodeList.size - 2) << 1) | 1;

		return internId;
	}

	// Bucket refers to a linked list of interns
	Intern intern;
	Node* node = &nodes[bucket >> 1];
	while(1) {
		intern = interns[node->intern];

		// Existing intern found
		if(hash == intern.hash && length == intern.length && memcmp(string, intern.string, length) == 0)
			return intern.id;

		// If the intern does not exist, then append the new intern to the end of the linked list
		if(node->next == 0) {
			int internId = addInternToList(string, length, hash);
			node->next = nodeList.size;
			listEnsureCapacity(&nodeList, sizeof(Node));
			nodes[nodeList.size].intern = internId;
			nodes[nodeList.size].next = 0;
			nodeList.size++;
			return internId;
		}

		node = &nodes[node->next];
	}
}