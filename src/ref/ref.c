#include "ref.h"

#include <memory.h>
#include <malloc.h>
#include <stdbool.h>

struct RefHeader {
    unsigned int refCount;
    struct DataType* dataType;
};

void refReleaseChildren(void* obj, struct DataType* dataType);

bool refIsCounted(struct DataType* dataType) {
    return dataType->type == DataTypePointer || dataType->type == DataTypeFixedArray || dataType->type == DataTypeVariableArray || dataType->type == DataTypeObject || dataType->type == DataTypeOpaque;
}

void* _refMallocRaw(size_t size) {
    unsigned allocationSize = size + sizeof(struct RefHeader);
    struct RefHeader* result = malloc(allocationSize);
    memset(result, 0, allocationSize);

    result->refCount = 1;
    result->dataType = 0;

    return result + 1;
}

void _refChangeType(void* obj, struct DataType* dataType) {
    if (!obj) {
        return;
    }

    struct RefHeader* header = (struct RefHeader*)obj - 1;
    refRelease(header->dataType);
    header->dataType = dataType;
    refRetain(dataType);
}

void* refMalloc(struct DataType* dataType) {
    unsigned allocationSize = dataTypeSize(dataType) + sizeof(struct RefHeader);
    struct RefHeader* result = malloc(allocationSize);
    memset(result, 0, allocationSize);

    result->refCount = 1;
    result->dataType = dataType;
    refRetain(dataType);

    return result + 1;
}

struct DynamicArray* refMallocArray(struct VariableArrayDataType* dataType, unsigned capacity) {
    unsigned allocationSize = dataTypeSize(dataType->subType) * capacity + 
        sizeof(struct DynamicArrayHeader) + 
        sizeof(struct RefHeader);
    
    struct RefHeader* result = malloc(allocationSize);
    memset(result, 0, allocationSize);

    result->refCount = 1;
    result->dataType = (struct DataType*)dataType;
    refRetain(dataType);

    struct DynamicArrayHeader* header = (struct DynamicArrayHeader*)(result + 1); 

    header->count = 0;
    header->capacity = capacity;

    return (struct DynamicArray*)header;
}

OString refMallocString(struct StringDataType* dataType, unsigned byteLength, char* dataSource) {
    // TODO support strings longer than 255 bytes
    OString result = malloc(byteLength + 1);
    result[0] = byteLength;
    memcpy(result + 1, dataSource, byteLength);
    return result;
}

void refRetain(void* obj) {
    if (!obj) {
        return;
    }

    struct RefHeader* header = (struct RefHeader*)obj - 1;
    header->refCount++;
}

void refFree(void* obj) {
    struct RefHeader* header = (struct RefHeader*)obj - 1;
    refReleaseChildren(obj, header->dataType);
    refRelease(header->dataType);
    header->dataType = NULL;
    free(header);
}

void refRelease(void* obj) {
    if (!obj) {
        return;
    }

    struct RefHeader* header = (struct RefHeader*)obj - 1;
    header->refCount--;

    if (header->refCount == 0) {
        refFree(obj);
    }
}

void refReleaseArray(void* firstElement, struct DataType* subType, unsigned count) {
    if (!refIsCounted(subType)) {
        return;
    }
 
    char* iterator = firstElement;
    size_t subTypeSize = dataTypeSize(subType);
    
    for (unsigned i = 0; i < count; ++i) {
        refReleaseChildren(iterator, subType);
        iterator += subTypeSize;
    }
}

void refReleaseObject(void* firstElement, struct ObjectDataType* objectType) {
    char* structStart = firstElement;

    struct ObjectSubType* end = &objectType->objectSubTypes->elements[0] + objectType->objectSubTypes->header.count;
    for (struct ObjectSubType* curr = &objectType->objectSubTypes->elements[0]; curr < end; ++curr) {
        refReleaseChildren(structStart + curr->offset, curr->type);
    }
}

void refReleaseChildren(void* obj, struct DataType* dataType) {
    switch (dataType->type) {
        case DataTypePointer:
            refRelease(*((void**)obj));
            break;
        case DataTypeFixedArray:
            refReleaseArray(obj, ((struct FixedArrayDataType*)dataType)->subType, ((struct FixedArrayDataType*)dataType)->elementCount);
            break;
        case DataTypeVariableArray:
            {
                struct DynamicArray* arr = (struct DynamicArray*)obj;
                refReleaseArray(&arr->data, ((struct VariableArrayDataType*)dataType)->subType, arr->header.count);
            }
            break;
        case DataTypeObject:
            refReleaseObject(obj, (struct ObjectDataType*)dataType);
            break;
        default:
            // do nothing for all other types
            break;
    }
}