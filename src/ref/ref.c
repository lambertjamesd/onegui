#include "ref.h"

#include <memory.h>
#include <malloc.h>
#include <stdbool.h>

struct RefHeader {
    unsigned int refCount;
    struct DataType* dataType;
};

void refReleaseType(void* obj, struct DataType* dataType);
void refReleaseChildren(void* obj, struct DataType* dataType);

bool refIsCounted(struct DataType* dataType) {
    if (dataType == NULL) {
        return false;
    }
    
    return dataType->type == DataTypePointer || dataType->type == DataTypeFixedArray || dataType->type == DataTypeDynamicArray || dataType->type == DataTypeObject || dataType->type == DataTypeOpaque;
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

struct DataType* refGetDataType(const void* obj) {
    if (!obj) {
        return 0;
    }

    struct RefHeader* header = (struct RefHeader*)obj - 1;
    return header->dataType;
}

int _refGetCount(const void* obj) {
    if (!obj) {
        return 0;
    }

    struct RefHeader* header = (struct RefHeader*)obj - 1;
    return header->refCount;
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

struct DynamicArray* refMallocArray(struct DynamicArrayDataType* dataType, unsigned capacity) {
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

OString refMallocString(struct StringDataType* dataType, unsigned byteLength, const char* dataSource) {
    int prefixLength = 1;
    int lenData = byteLength;

    while (lenData > 0x7f) {
        ++prefixLength;
        lenData >>= 7;
    }

    struct RefHeader* header = malloc(byteLength + prefixLength + sizeof(struct RefHeader));
    header->refCount = 1;
    header->dataType = (struct DataType*)dataType;
    refRetain(dataType);

    char* result = (char*)(header + 1);

    int lengthIndex = 0;
    lenData = byteLength;

    while (lengthIndex < prefixLength) {
        result[lengthIndex] = 0x7f & (byteLength >> (7 * (prefixLength - lengthIndex - 1)));
        ++lengthIndex;
    }

    memcpy(result + prefixLength, dataSource, byteLength);
    return result;
}

void* refRetain(void* obj) {
    if (!obj) {
        return NULL;
    }

    struct RefHeader* header = (struct RefHeader*)obj - 1;
    header->refCount++;
    return obj;
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
        refReleaseType(iterator, subType);
        iterator += subTypeSize;
    }
}

void refReleaseObject(void* firstElement, struct ObjectDataType* objectType) {
    char* structStart = firstElement;

    struct ObjectSubType* end = &objectType->objectSubTypes->elements[0] + objectType->objectSubTypes->header.count;
    for (struct ObjectSubType* curr = &objectType->objectSubTypes->elements[0]; curr < end; ++curr) {
        if (!curr->type) {
            continue;
        }

        refReleaseType(structStart + curr->offset, curr->type);
    }
}

void refReleaseType(void* obj, struct DataType* dataType) {
    if (!dataType) {
        return;
    }

    switch (dataType->type) {
        case DataTypePointer:
        case DataTypeString:
            refRelease(*((void**)(obj)));
            break;
        case DataTypeFixedArray:
            refReleaseArray(obj, ((struct FixedArrayDataType*)dataType)->subType, ((struct FixedArrayDataType*)dataType)->elementCount);
            break;
        case DataTypeDynamicArray:
            {
                struct DynamicArray* arr = (struct DynamicArray*)obj;
                refReleaseArray(&arr->data, ((struct DynamicArrayDataType*)dataType)->subType, arr->header.count);
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

void refReleaseChildren(void* obj, struct DataType* dataType) {
    if (!dataType) {
        return;
    }

    switch (dataType->type) {
        case DataTypeFixedArray:
            refReleaseArray(obj, ((struct FixedArrayDataType*)dataType)->subType, ((struct FixedArrayDataType*)dataType)->elementCount);
            break;
        case DataTypeDynamicArray:
            {
                struct DynamicArray* arr = (struct DynamicArray*)obj;
                refReleaseArray(&arr->data, ((struct DynamicArrayDataType*)dataType)->subType, arr->header.count);
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

uint64_t refSize(const void* obj) {
    struct DataType* type = refGetDataType(obj);

    if (type->type == DataTypeDynamicArray) {
        struct DynamicArrayDataType* arrayType = (struct DynamicArrayDataType*)type;
        struct DynamicArrayHeader* asArray = (struct DynamicArrayHeader*)obj;

        return sizeof(struct DynamicArrayHeader) + asArray->capacity * dataTypeSize(arrayType->subType);
    } else {
        return dataTypeSize(type);
    }
}