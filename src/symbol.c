typedef struct {
	int name;
} Symbol;

// SymTable is a map of int (intern) to int (symbol index)
// Each bucket is an int array that consists of symbol indices
// Initial bucket size of 8
// SymTable data
// Symbol tables are persistent

typedef struct {
	int intern;
	int next;
} SymTableNode;



typedef struct {
	int bucketsSize;
	int bucketsCapacity;
	int* buckets;
} SymTable;