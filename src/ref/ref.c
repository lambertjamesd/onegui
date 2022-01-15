#include "ref.h"

#include <memory.h>
#include <stdbool.h>

struct RefHeader {
    unsigned int refCount;
    struct DataType* dataType;
};

void ref_release_children(void* obj, struct DataType* dataType);

bool is_ref_counted(struct DataType* dataType) {
    return dataType->type == DataTypePointer || dataType->type == DataTypeFixedArray || dataType->type == DataTypeVariableArray || dataType->type == DataTypeObject || dataType->type == DataTypeOpaque;
}

void* _ref_malloc_raw(size_t size) {
    unsigned allocationSize = size + sizeof(struct RefHeader);
    struct RefHeader* result = malloc(allocationSize);
    memset(result, 0, allocationSize);

    result->refCount = 1;
    result->dataType = 0;

    return result + 1;
}

struct DynamicArray* _ref_malloca_array_raw(size_t elementSize, unsigned capacity) {
    unsigned allocationSize = elementSize * capacity + 
        sizeof(struct DynamicArrayHeader) + 
        sizeof(struct RefHeader);
    
    struct RefHeader* result = malloc(allocationSize);
    memset(result, 0, allocationSize);

    result->refCount = 1;
    result->dataType = 0;

    struct DynamicArrayHeader* header = (struct DynamicArrayHeader*)(result + 1); 

    header->count = 0;
    header->capacity = capacity;

    return (struct DynamicArray*)header;
}

void* _ref_change_type(void* obj, struct DataType* dataType) {
    if (!obj) {
        return;
    }

    struct RefHeader* header = (struct RefHeader*)obj - 1;
    ref_release(header->dataType);
    header->dataType = dataType;
    ref_retain(dataType);
}

void* ref_malloc(struct DataType* dataType) {
    unsigned allocationSize = dataTypeSize(dataType) + sizeof(struct RefHeader);
    struct RefHeader* result = malloc(allocationSize);
    memset(result, 0, allocationSize);

    result->refCount = 1;
    result->dataType = dataType;
    ref_retain(dataType);

    return result + 1;
}

struct DynamicArray* ref_malloc_array(struct VariableArrayDataType* dataType, unsigned capacity) {
    unsigned allocationSize = dataTypeSize(dataType->subType) * capacity + 
        sizeof(struct DynamicArrayHeader) + 
        sizeof(struct RefHeader);
    
    struct RefHeader* result = malloc(allocationSize);
    memset(result, 0, allocationSize);

    result->refCount = 1;
    result->dataType = dataType;
    ref_retain(dataType);

    struct DynamicArrayHeader* header = (struct DynamicArrayHeader*)(result + 1); 

    header->count = 0;
    header->capacity = capacity;

    return (struct DynamicArray*)header;
}

OString ref_malloc_str(struct StringDataType* dataType, unsigned byteLength, char* dataSource) {
    // TODO support strings longer than 255 bytes
    OString result = malloc(byteLength + 1);
    result[0] = byteLength;
    memcpy(result + 1, dataSource, byteLength);
    return result;
}

void ref_retain(void* obj) {
    if (!obj) {
        return;
    }

    struct RefHeader* header = (struct RefHeader*)obj - 1;
    header->refCount++;
}

void ref_free(void* obj) {
    struct RefHeader* header = (struct RefHeader*)obj - 1;
    ref_release_children(obj, header->dataType);
    ref_release(header->dataType);
    header->dataType = NULL;
    free(header);
}

void ref_release(void* obj) {
    if (!obj) {
        return;
    }

    struct RefHeader* header = (struct RefHeader*)obj - 1;
    header->refCount--;

    if (header->refCount == 0) {
        ref_free(obj);
    }
}

void ref_release_array(void* firstElement, struct DataType* subType, unsigned count) {
    if (!is_ref_counted(subType)) {
        return;
    }
 
    char* iterator = firstElement;
    size_t subTypeSize = dataTypeSize(subType);
    
    for (unsigned i = 0; i < count; ++i) {
        ref_release_children(iterator, subType);
        iterator += subTypeSize;
    }
}

void ref_release_object(void* firstElement, struct ObjectDataType* objectType) {
    char* structStart = firstElement;

    struct ObjectSubType* end = objectType->objectSubTypes + objectType->objectSubTypes->header.count;
    for (struct ObjectSubType* curr = objectType->objectSubTypes->elements; curr < end; ++curr) {
        ref_release_children(structStart + curr->offset, &curr->type);
    }
}

void ref_release_children(void* obj, struct DataType* dataType) {
    switch (dataType->type) {
        case DataTypePointer:
            ref_release(*((void**)obj));
            break;
        case DataTypeFixedArray:
            ref_release_array(obj, ((struct FixedArrayDataType*)dataType)->subType, ((struct FixedArrayDataType*)dataType)->elementCount);
            break;
        case DataTypeVariableArray:
            {
                struct DynamicArray* arr = (struct DynamicArrayHeader*)obj;
                ref_release_array(&arr->data, ((struct VariableArrayDataType*)dataType)->subType, arr->header.count);
            }
            break;
        case DataTypeObject:
            ref_release_object(obj, (struct ObjectDataType*)dataType);
            break;
    }
}