#ifndef __ONEGUI_BASIC_TYPES_H__
#define __ONEGUI_BASIC_TYPES_H__

#include "../types/types.h"

struct BasicDataTypes {
    struct DataType* primitiveTypes[DataTypePrimitiveCount];

    struct ObjectDataType* objectDataType;
    struct ObjectDataType* objectSubType;
    struct DynamicArrayDataType* objectSubTypeArray;
    struct PointerDataType* pointerToObjectSubTypeArray;
    struct ObjectDataType* primitiveDataType;
    struct ObjectDataType* pointerDataType;
    struct StringDataType* stringDataType;
    struct DataType* unknownType;
    struct PointerDataType* pointerToUnknownType;
    struct ObjectDataType* dynamicArrayDataType;
    struct ObjectDataType* fixedArrayDataType;
};

void basicTypesInit(struct BasicDataTypes* into);
void basicTypesDestroy(struct BasicDataTypes* basicTypes);

struct DataType* basicTypesGetPrimitive(struct BasicDataTypes* basicTypes, enum DataTypeType type);
struct ObjectDataType* basicTypesNewObject(struct BasicDataTypes* basicTypes, size_t size, unsigned elementCount);
void basicTypesAppendSubType(struct BasicDataTypes* basicTypes, struct ObjectDataType* onto, char* cStrName, struct DataType* subType, unsigned offset);
struct PointerDataType* basicTypesNewPointerType(struct BasicDataTypes* basicTypes, struct DataType* pointTo);
struct PointerDataType* basicTypesNewWeakPointerType(struct BasicDataTypes* basicTypes, struct DataType* pointTo);
struct DynamicArrayDataType* basicTypesNewVariableArray(struct BasicDataTypes* basicTypes, struct DataType* elementType);
struct FixedArrayDataType* basicTypesNewFixedArray(struct BasicDataTypes* basicTypes, struct DataType* elementType, unsigned size);

#endif