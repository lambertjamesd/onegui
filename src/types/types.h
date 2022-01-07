#ifndef __ONEGUI_TYPES_H__
#define __ONEGUI_TYPES_H__

#include <stddef.h>

typedef unsigned int GCRef;

#define ARRAY_SIZE_VARIABLE 0

enum DataTypeType {
    DataTypeNull,
    
    DataTypeInt,
    DataTypeUInt,
    DataTypeFloat,

    DataTypePointer,
    DataTypeArray,
    DataTypeObject,

    DataTypeFunction,

    DataTypeOpaque,

    // references a DataType
    DataTypeDataType,
    
    DataTypeGCRef,
};

struct DataSubType;

struct DataType {
    enum DataTypeType type;
    // The number of elements for type
    // DataTypeObject
    // 0 for variable sized arrays
    // a positive value for fixed size arrays
    // specifys the size of integer or float in bytes for number types
    unsigned int size;
    union {
        GCRef arrayType;
        GCRef pointerType;
        GCRef objectSubTypes;
    };
};

struct DataSubType {
    char* name;
    struct DataType type;
    unsigned offset;
};

struct AnyObjectReference {
    struct DataType* dataType;
    void* data;
};

struct DynamicArrayHeader {
    unsigned count;
    unsigned capacity;
};

struct DynamicArray {
    struct DynamicArrayHeader header;
    char* data[];
};

size_t dataTypeSize(struct DataType* type);

#endif