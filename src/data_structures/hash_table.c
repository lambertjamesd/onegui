#include "hash_table.h"
#include "../types/type_builder.h"

struct DataType* _gHashTableType;
struct DynamicArrayDataType* _gHashTableEntryTables[4];
struct DynamicArrayDataType* _gHashTableBinArrayType;

#define INITIAL_CAPACITY 8

int _hashTableArrayTypeIndex(enum HashTableFlags flags) {
    int result = 0;

    if (flags & HashTableFlagsRetainKey) {
        result |= 2;
    }

    if (flags & HashTableFlagsRetainValue) {
        result |= 1;
    }

    return result;
}

struct DynamicArrayDataType* _hashTableBuildHashEntryArrayType(enum HashTableFlags flags) {
    struct DataType* elementType = typeBuilderNewObject(sizeof(struct HashTable), 3);

    TYPE_BUILDER_APPEND_SUB_TYPE(
        elementType,
        struct HashEntry,
        next,
        typeBuilderGetWeakPointerToUnknown()
    );
    // TODO handle big endian correctly
    TYPE_BUILDER_APPEND_SUB_TYPE(
        elementType,
        struct HashEntry,
        key,
        (flags & HashTableFlagsRetainKey) ? typeBuilderGetPointerToUnknown() : typeBuilderGetPrimitive(DataTypeUInt64)
    );
    // TODO handle big endian correctly
    TYPE_BUILDER_APPEND_SUB_TYPE(
        elementType,
        struct HashEntry,
        value,
        (flags & HashTableFlagsRetainValue) ? typeBuilderGetPointerToUnknown() : typeBuilderGetPrimitive(DataTypeUInt64)
    );

    struct DynamicArrayDataType* arrayType = typeBuilderNewVariableArray(elementType);
    refRelease(elementType);

    return arrayType;
}

void hashTableTypeInit() {
    _gHashTableType = typeBuilderNewObject(sizeof(struct HashTable), 7);
    TYPE_BUILDER_APPEND_SUB_TYPE(
        _gHashTableType, 
        struct HashTable,
        equalityCompare, 
        typeBuilderGetWeakPointerToUnknown()
    );
    TYPE_BUILDER_APPEND_SUB_TYPE(
        _gHashTableType, 
        struct HashTable,
        hashFunction, 
        typeBuilderGetWeakPointerToUnknown() 
    );
    TYPE_BUILDER_APPEND_SUB_TYPE(
        _gHashTableType, 
        struct HashTable,
        entryArray, 
        typeBuilderGetPointerToUnknown() 
    );
    TYPE_BUILDER_APPEND_SUB_TYPE(
        _gHashTableType, 
        struct HashTable,
        binArray, 
        typeBuilderGetPointerToUnknown() 
    );
    TYPE_BUILDER_APPEND_SUB_TYPE(
        _gHashTableType, 
        struct HashTable,
        firstFreeEntry, 
        typeBuilderGetWeakPointerToUnknown() 
    );
    TYPE_BUILDER_APPEND_SUB_TYPE(
        _gHashTableType, 
        struct HashTable,
        entryCount, 
        typeBuilderGetPrimitive(DataTypeUInt32) 
    );
    TYPE_BUILDER_APPEND_SUB_TYPE(
        _gHashTableType, 
        struct HashTable,
        hashTableFlags, 
        typeBuilderGetPrimitive(DataTypeUInt32) 
    );

    _gHashTableEntryTables[_hashTableArrayTypeIndex(0)] = _hashTableBuildHashEntryArrayType(0);
    _gHashTableEntryTables[_hashTableArrayTypeIndex(HashTableFlagsRetainKey)] = _hashTableBuildHashEntryArrayType(HashTableFlagsRetainKey);
    _gHashTableEntryTables[_hashTableArrayTypeIndex(HashTableFlagsRetainValue)] = _hashTableBuildHashEntryArrayType(HashTableFlagsRetainValue);
    _gHashTableEntryTables[_hashTableArrayTypeIndex(HashTableFlagsRetainKey | HashTableFlagsRetainValue)] = _hashTableBuildHashEntryArrayType(HashTableFlagsRetainKey | HashTableFlagsRetainValue);

    _gHashTableBinArrayType = typeBuilderNewVariableArray(typeBuilderGetWeakPointerToUnknown());
}

struct HashTable* hashTableNew(HashEntryEqualityCompare equalityCompare, HashEntryHashFunction hashFunction, enum HashTableFlags flags) {
    struct HashTable* result = refMalloc(_gHashTableType);

    result->equalityCompare = equalityCompare;
    result->hashFunction = hashFunction;

    result->entryArray = (struct HashEntryArray*)refMallocArray(_gHashTableEntryTables[_hashTableArrayTypeIndex(flags)], INITIAL_CAPACITY);
    result->entryArray->header.count = result->entryArray->header.capacity;
    result->binArray = (struct HashBinArray*)refMallocArray(_gHashTableBinArrayType, INITIAL_CAPACITY * 2);
    result->binArray->header.count = result->binArray->header.capacity;

    for (unsigned i = 0; i + 1 < result->entryArray->header.count; ++i) {
        result->entryArray->table[i].next = &result->entryArray->table[i + 1];
    }
    result->firstFreeEntry = &result->entryArray->table[0];

    result->entryCount = 0;
    result->hashTableFlags = flags;

    return result;
}

int _hashTableGetBinIndex(struct HashTable* table, uint64_t key) {
    return table->hashFunction(key) % table->binArray->header.count;
}

struct HashEntry* _hashTableFindCurrentEntry(struct HashTable* table, uint64_t key, int* binOutput) {
    int bin = _hashTableGetBinIndex(table, key);
    if (binOutput) {
        *binOutput = bin;
    }
    struct HashEntry* curr = table->binArray->table[bin];

    while (curr) {
        if (table->equalityCompare(curr->key, key)) {
            return curr;
        }
        curr = curr->next;
    }

    return NULL;
}

void _hashTableSetNewSize(struct HashTable* table, int newEntryCount, int newBinCount) {
    struct HashEntryArray* prevEntryArray = table->entryArray;
    struct HashBinArray* prevBinArray = table->binArray;
    
    // reset table with new size
    table->entryArray = (struct HashEntryArray*)refMallocArray(_gHashTableEntryTables[_hashTableArrayTypeIndex(table->hashTableFlags)], newEntryCount);
    table->entryArray->header.count = table->entryArray->header.capacity;
    table->binArray = (struct HashBinArray*)refMallocArray(_gHashTableBinArrayType, newBinCount);
    table->binArray->header.count = table->binArray->header.capacity;
    table->entryCount = 0;

    // read all elements
    for (unsigned i = 0; i < prevBinArray->header.count; ++i) {
        struct HashEntry* curr = prevBinArray->table[i];

        while (curr) {
            hashTableSet(table, curr->key, curr->value);
            curr = curr->next;
        }
    }

    // clean up old array
    refRelease(prevEntryArray);
    refRelease(prevBinArray);
}

void hashTableSet(struct HashTable* table, uint64_t key, uint64_t value) {
    int bin;
    struct HashEntry* prevEntry = _hashTableFindCurrentEntry(table, key, &bin);

    if (prevEntry) {
        if (table->hashTableFlags & HashTableFlagsRetainValue) {
            refRetain((void*)value);
            refRelease((void*)prevEntry->value);
        }
        prevEntry->value = value;
        return;
    }

    if (!table->firstFreeEntry) {
        _hashTableSetNewSize(table, table->entryArray->header.count * 2, table->binArray->header.count * 2);
    }

    struct HashEntry* newEntry = table->firstFreeEntry;
    table->firstFreeEntry = newEntry->next;
    ++table->entryCount;

    if (table->hashTableFlags & HashTableFlagsRetainKey) {
        refRetain((void*)key);
    }
    newEntry->key = key;
    if (table->hashTableFlags & HashTableFlagsRetainValue) {
        refRetain((void*)value);
    }
    newEntry->value = value;

    newEntry->next = table->binArray->table[bin];
    table->binArray->table[bin] = newEntry;
}

bool hashTableGet(struct HashTable* table, uint64_t key, uint64_t* result) {
    struct HashEntry* entry = _hashTableFindCurrentEntry(table, key, NULL);

    if (entry) {
        if (result) {
            *result = entry->value;
        }
        return true;
    }

    return false;
}

void hashTableDelete(struct HashTable* table, uint64_t key) {
    int bin = _hashTableGetBinIndex(table, key);

    struct HashEntry* prevEntry = NULL;
    struct HashEntry* curr = table->binArray->table[bin];

    while (curr) {
        if (table->equalityCompare(curr->key, key)) {
            if (prevEntry) {
                prevEntry->next = curr->next;
            } else {
                table->binArray->table[bin] = curr->next;
            }

            curr->next = table->firstFreeEntry;
            table->firstFreeEntry = curr;

            if (table->hashTableFlags & HashTableFlagsRetainKey) {
                refRelease((void*)curr->key);
            }
            curr->key = 0;
            if (table->hashTableFlags & HashTableFlagsRetainValue) {
                refRelease((void*)curr->value);
            }
            curr->value = 0;
            --table->entryCount;
            break;;
        }
        prevEntry = curr;
        curr = curr->next;
    }

    if (table->entryCount < table->entryArray->header.count / 4 && table->entryArray->header.count > INITIAL_CAPACITY) {
        _hashTableSetNewSize(table, table->entryArray->header.count / 2, table->binArray->header.count / 2);
    }
}
