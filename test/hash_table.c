#include "../src/data_structures/hash_table.h"

#include "test.h"
#include "../src/ref/ref_privite.h"
#include "../src/types/string.h"

void testHashTable() {
    struct HashTable* numberHashTable = hashTableNew(hashTableBasicEquality, hashTableIntegerHash, 0);

    TEST_ASSERT(!hashTableGet(numberHashTable, 10, NULL));
    TEST_ASSERT(hashTableSize(numberHashTable) == 0);
    
    uint64_t storedValue;
    hashTableSet(numberHashTable, 10, 100);
    TEST_ASSERT(hashTableGet(numberHashTable, 10, &storedValue));
    TEST_ASSERT(storedValue == 100);
    TEST_ASSERT(hashTableSize(numberHashTable) == 1);

    hashTableSet(numberHashTable, 10, 50);
    TEST_ASSERT(hashTableGet(numberHashTable, 10, &storedValue));
    TEST_ASSERT(storedValue == 50);
    TEST_ASSERT(hashTableSize(numberHashTable) == 1);

    hashTableSet(numberHashTable, 20, 30);
    TEST_ASSERT(hashTableGet(numberHashTable, 20, &storedValue));
    TEST_ASSERT(storedValue == 30);
    TEST_ASSERT(hashTableSize(numberHashTable) == 2);

    hashTableDelete(numberHashTable, 10);
    TEST_ASSERT(!hashTableGet(numberHashTable, 10, NULL));
    TEST_ASSERT(hashTableSize(numberHashTable) == 1);

    refRelease(numberHashTable);
}

void testHashTableKeyReference() {
    struct HashTable* stringHashTable = HASH_TABLE_NEW_STR_STR();

    OString foo = OSTR_NEW_FROM_CSTR("foo");
    OString bar = OSTR_NEW_FROM_CSTR("bar");
    OString fizz = OSTR_NEW_FROM_CSTR("fizz");

    OString fooCopy = OSTR_NEW_FROM_CSTR("foo");

    TEST_ASSERT(_refGetCount(foo) == 1);
    TEST_ASSERT(_refGetCount(bar) == 1);
    TEST_ASSERT(_refGetCount(fizz) == 1);

    TEST_ASSERT(_refGetCount(fooCopy) == 1);

    hashTableSetKV(stringHashTable, foo, bar);
    TEST_ASSERT(hashTableSize(stringHashTable) == 1);

    TEST_ASSERT(_refGetCount(foo) == 2);
    TEST_ASSERT(_refGetCount(bar) == 2);

    OString strOut;

    TEST_ASSERT(hashTableGetKV(stringHashTable, fooCopy, (void**)&strOut));
    TEST_ASSERT(strOut == bar);

    hashTableDeleteK(stringHashTable, fooCopy);
    TEST_ASSERT(hashTableSize(stringHashTable) == 0);

    TEST_ASSERT(_refGetCount(foo) == 1);
    TEST_ASSERT(_refGetCount(bar) == 1);

    hashTableSetKV(stringHashTable, foo, bar);
    hashTableSetKV(stringHashTable, fizz, foo);

    TEST_ASSERT(_refGetCount(foo) == 3);
    TEST_ASSERT(_refGetCount(bar) == 2);
    TEST_ASSERT(_refGetCount(fizz) == 2);

    refRelease(stringHashTable);

    TEST_ASSERT(_refGetCount(foo) == 1);
    TEST_ASSERT(_refGetCount(bar) == 1);

    refRelease(foo);
    refRelease(bar);
    refRelease(fizz);
    refRelease(fooCopy);
}