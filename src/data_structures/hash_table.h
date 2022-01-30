#ifndef __ONEGUI_HASH_TABLE_H__
#define __ONEGUI_HASH_TABLE_H__

#include "../types/types.h"
#include "../ref/ref.h"

#include <stdbool.h>

struct HashEntry {
    struct HashEntry* next;
    uint64_t key;
    uint64_t value;
};

struct HashEntryArray {
    struct DynamicArrayHeader header;
    struct HashEntry table[];
};

struct HashBinArray {
    struct DynamicArrayHeader header;
    struct HashEntry* table[];
};

typedef bool (*HashEntryEqualityCompare)(uint64_t a, uint64_t b);
typedef uint64_t (*HashEntryHashFunction)(uint64_t input);

enum HashTableFlags {
    // interpret key as a pointer and retain references to them
    HashTableFlagsRetainKey = (1 << 0),
    // interpret value as a pointer and retain references to them
    HashTableFlagsRetainValue = (1 << 1),
};

struct HashTable {
    HashEntryEqualityCompare equalityCompare;
    HashEntryHashFunction hashFunction;
    struct HashEntryArray* entryArray;
    struct HashBinArray* binArray;
    struct HashEntry* firstFreeEntry;
    uint32_t entryCount;
    uint32_t hashTableFlags;
};

struct HashTableIterator {
    struct HashTable* forTable;
    uint32_t currentBin;
    struct HashEntry* currentEntry;
};

#define HASH_TABLE_NEW_STR_STR()    hashTableNew(hashTableStringEquality, hashTableStringHash, HashTableFlagsRetainKey | HashTableFlagsRetainValue)

struct HashTable* hashTableNew(HashEntryEqualityCompare equalityCompare, HashEntryHashFunction hashFunction, enum HashTableFlags flags);

void hashTableTypeInit();

void hashTableSet(struct HashTable* table, uint64_t key, uint64_t value);
bool hashTableGet(struct HashTable* table, uint64_t key, uint64_t* result);
void hashTableDelete(struct HashTable* table, uint64_t key);

// these are helper functions for hash tables that 
// store pointers to objects
void hashTableSetK(struct HashTable* table, void* key, uint64_t value);
bool hashTableGetK(struct HashTable* table, void* key, uint64_t* result);
void hashTableDeleteK(struct HashTable* table, void* key);

void hashTableSetKV(struct HashTable* table, void* key, void* value);
bool hashTableGetKV(struct HashTable* table, void* key, void** result);

void hashTableSetV(struct HashTable* table, uint64_t key, void* value);
bool hashTableGetV(struct HashTable* table, uint64_t key, void** result);

uint32_t hashTableSize(struct HashTable* table);

uint64_t hashTableIntegerHash(uint64_t key);
bool hashTableBasicEquality(uint64_t a, uint64_t b);

uint64_t hashTableStringHash(uint64_t key);
bool hashTableStringEquality(uint64_t a, uint64_t b);

void hashTableIteratorInit(struct HashTableIterator* out, struct HashTable* table);
struct HashEntry* hashTableIteratorCurrent(struct HashTableIterator* it);
void hashTableIteratorNext(struct HashTableIterator* it);

#endif