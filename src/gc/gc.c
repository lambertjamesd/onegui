#include "gc.h"
#include "memory.h"


#define ARRAY_HEADER_SIZE       (sizeof(struct DynamicArrayHeader))

#define INITIAL_ENTRY_COUNT     1024
#define INITIAL_PINNED_COUNT    32

// Minimum of 1 MB
#define MIN_INITIAL_SIZE        (1024 * 1024)

struct GCEntry {
    union {
        GCRef typeEntry;
        GCRef nextFreeEntry;
    };
    void *ptr;
    unsigned uniqueID;
};

struct GCEntryArray {
    struct DynamicArrayHeader header;
    struct GCEntry arr[];
};

struct PinnedEntryArray {
    struct DynamicArrayHeader header;
    GCRef arr[];
};

struct GC {
    unsigned nextID;
    struct GCEntryArray* entries;

    GCRef firstFreeEntry;

    GCRef pinnedEntries;

    char* heapA;
    char* heapB;
    char* nextPointer;
    char* currentHeapEnd;
};

struct GC gGC;

void* gc_malloc_raw(unsigned size) {
    unsigned remaining = gGC.currentHeapEnd - gGC.nextPointer;

    if (remaining < size) {
        gc_collect();

        remaining = gGC.currentHeapEnd - gGC.nextPointer;
    }

    if (remaining < size) {
        // TODO grow
    }

    void* result = gGC.nextPointer;
    gGC.nextPointer += size;
    return result;
}

void gc_init(unsigned initialSize) {
    if (initialSize < MIN_INITIAL_SIZE) {
        initialSize = MIN_INITIAL_SIZE;
    }

    initialSize = (initialSize + 0x7) & ~0x7;
    gGC.heapA = (char*)memorySize();
    gGC.nextPointer = gGC.heapA;
    gGC.heapB = gGC.heapA + initialSize;
    gGC.currentHeapEnd = gGC.heapB;
    memoryGrow(initialSize * 2);

    gGC.nextID = 1;
    gGC.entries = gc_malloc_raw(ARRAY_HEADER_SIZE + INITIAL_ENTRY_COUNT * sizeof(struct GCEntry));
    gGC.entries->header.capacity = INITIAL_ENTRY_COUNT;
    gGC.pinnedEntries = gc_malloc_raw(ARRAY_HEADER_SIZE + INITIAL_PINNED_COUNT * sizeof(GCRef));
    gGC.pinnedEntries->header.capacity = INITIAL_PINNED_COUNT;

    gGC.firstFreeEntry = 1;
    for (unsigned i = 1; i + 1 < INITIAL_ENTRY_COUNT; ++i) {
        gGC.entries->arr[i].nextFreeEntry = i + 1;
    }
}

GCRef gc_malloc_size(GCRef typeEntry, unsigned size) {
    void* ptr = gc_malloc_raw(size);

    if (!ptr) {
        return 0;
    }

    if (!gGC.firstFreeEntry) {
        // TODO allocate more entries
    }

    GCRef result = gGC.firstFreeEntry;
    gGC.firstFreeEntry = gGC.entries->arr[gGC.firstFreeEntry].nextFreeEntry;

    struct GCEntry* entry = &gGC.entries[result];

    entry->ptr = ptr;
    entry->typeEntry = typeEntry;
    entry->uniqueID = gGC.nextID;
    ++gGC.nextID;
    return result;
}

GCRef gc_malloc_array(GCRef arrayType, unsigned count) {
    struct DataType* dataType = gc_lookup(arrayType);
    
    if (!dataType) {
        return 0;
    }

    if (dataType->type != DataTypeArray) {
        return 0;
    }

    if (dataType->size && dataType->size != count) {
        return 0;
    }

    struct DataType* subType = gc_lookup(dataType->arrayType);

    unsigned dataSize = dataTypeSize(subType) * count;

    if (dataType->size == ARRAY_SIZE_VARIABLE) {
        dataSize += ARRAY_HEADER_SIZE;
    }

    GCRef result = gc_malloc_size(arrayType, dataSize);

    if (dataType->size == ARRAY_SIZE_VARIABLE) {
        struct DynamicArray* arrayResult = gc_lookup(result);
        arrayResult->header.capacity = count;
    }

    return result;
}

GCRef gc_malloc(GCRef typeEntry) {
    struct DataType* dataType = gc_lookup(typeEntry);
    
    if (!dataType) {
        return 0;
    }

    return gc_malloc_size(typeEntry, dataTypeSize(dataType));
}

void* gc_lookup(GCRef entry) {
    return gGC.entries->arr[entry].ptr;
}

void gc_collect() {

}