#ifndef __ONEGUI_TYPES_H__
#define __ONEGUI_TYPES_H__

#include <stddef.h>

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
    enum DataTypeType type;
};

struct PrimitiveDataType {
    enum DataTypeType type;
};

struct StringDataType {
    enum DataTypeType type;
};

struct FixedArrayDataType {
    enum DataTypeType type;
    struct DataType* subType;
    unsigned int elementCount;
};

struct VariableArrayDataType {
    enum DataTypeType type;
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
    enum DataTypeType type;
    struct DataType* subType;
};

struct ObjectSubTypeArray;

struct ObjectDataType {
    enum DataTypeType type;
    size_t byteSize;
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