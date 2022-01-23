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

struct HashTable* hashTableNew(HashEntryEqualityCompare equalityCompare, HashEntryHashFunction hashFunction, enum HashTableFlags flags);

void hashTableTypeInit();

void hashTableSet(struct HashTable* table, uint64_t key, uint64_t value);
bool hashTableGet(struct HashTable* table, uint64_t key, uint64_t* result);
void hashTableDelete(struct HashTable* table, uint64_t key);

#endif