#ifndef __ONEGUI_TYPES_H__
#define __ONEGUI_TYPES_H__

#include <stddef.h>
#include <stdint.h>

#define ARRAY_SIZE_VARIABLE 0

enum DataTypeType {
    DataTypeNull,
    
    DataTypeInt8,
    DataTypeUInt8,
    DataTypeInt16,
    DataTypeUInt16,
    DataTypeInt32,
    DataTypeUInt32,
    DataTypeInt64,
    DataTypeUInt64,
    DataTypeFloat32,
    DataTypeFloat64,

    DataTypePrimitiveCount,
    
    // same as pointer but not reference counted
    DataTypeWeakPointer = DataTypePrimitiveCount,
    // Ref counted
    DataTypePointer,

    DataTypeFixedArray,
    DataTypeVariableArray,
    DataTypeObject,

    // Used to allow pointers to 
    // point to an unknown type
    DataTypeUnknown,

    DataTypeString,

    DataTypeFunction,

    // TODO, potentially replace with a byte array
    DataTypeOpaque,
};

typedef char* OString;

struct DataType {
    uint32_t type;
};

struct PrimitiveDataType {
    uint32_t type;
};

struct StringDataType {
    uint32_t type;
};

struct FixedArrayDataType {
    uint32_t type;
    uint32_t elementCount;
    struct DataType* subType;
};

struct DynamicArrayDataType {
    uint32_t type;
    struct DataType* subType;
};

struct DynamicArrayHeader {
    unsigned count;
    unsigned capacity;
};

struct DynamicArray {
    struct DynamicArrayHeader header;
    char* data[];
};

struct PointerDataType {
    uint32_t type;
    struct DataType* subType;
};

struct ObjectSubTypeArray;

struct ObjectDataType {
    uint32_t type;
    uint32_t byteSize;
    struct ObjectSubTypeArray* objectSubTypes;
};

struct ObjectSubType {
    OString name;
    struct DataType* type;
    unsigned offset;
};

struct ObjectSubTypeArray {
    struct DynamicArrayHeader header;
    struct ObjectSubType elements[];
};

size_t dataTypeSize(struct DataType* type);

#endif