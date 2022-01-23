#include "type_builder.h"

#include "../ref/basic_types.h"

struct BasicDataTypes _gBasicDataTypes;
struct DataType* _gUnknownWeakPointer;

void typeBuilderInit() {
    basicTypesInit(&_gBasicDataTypes);
}

struct DataType* typeBuilderGetPrimitive(enum DataTypeType type) {
    return basicTypesGetPrimitive(&_gBasicDataTypes, type);
}

struct DataType* typeBuilderNewObject(size_t size, unsigned elementCount) {
    return (struct DataType*)basicTypesNewObject(&_gBasicDataTypes, size, elementCount);
}

void typeBuilderAppendSubType(struct DataType* onto, char* cStrName, struct DataType* subType, unsigned offset) {
    if (onto->type != DataTypeObject) {
        return;
    }

    basicTypesAppendSubType(&_gBasicDataTypes, (struct ObjectDataType*)onto, cStrName, subType, offset);
}

struct DataType* typeBuilderNewPointerType(struct DataType* pointTo) {
    return (struct DataType*)basicTypesNewPointerType(&_gBasicDataTypes, pointTo);
}

struct DataType* typeBuilderNewWeakPointerType(struct DataType* pointTo) {
    return (struct DataType*)basicTypesNewWeakPointerType(&_gBasicDataTypes, pointTo);
}

struct DynamicArrayDataType* typeBuilderNewVariableArray(struct DataType* elementType) {
    return basicTypesNewVariableArray(&_gBasicDataTypes, elementType);
}

struct DataType* typeBuilderNewFixedArray(struct DataType* elementType, unsigned size) {
    return (struct DataType*)basicTypesNewFixedArray(&_gBasicDataTypes, elementType, size);
}

struct DataType* typeBuilderGetPointerToUnknown() {
    return (struct DataType*)_gBasicDataTypes.pointerToUnknownType;
}

struct DataType* typeBuilderGetWeakPointerToUnknown() {
    return _gUnknownWeakPointer;
}