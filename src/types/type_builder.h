#ifndef __ONEGUI_TYPE_BUILDER_H__
#define __ONEGUI_TYPE_BUILDER_H__

#include "types.h"

void typeBuilderInit();

struct DataType* typeBuilderGetPrimitive(enum DataTypeType type);
struct DataType* typeBuilderNewObject(size_t size, unsigned elementCount);
void typeBuilderAppendSubType(struct DataType* onto, char* cStrName, struct DataType* subType, unsigned offset);
struct DataType* typeBuilderNewPointerType(struct DataType* pointTo);
struct DataType* typeBuilderNewWeakPointerType(struct DataType* pointTo);
struct DynamicArrayDataType* typeBuilderNewVariableArray(struct DataType* elementType);
struct DataType* typeBuilderNewFixedArray(struct DataType* elementType, unsigned size);
struct DataType* typeBuilderGetPointerToUnknown();
struct DataType* typeBuilderGetWeakPointerToUnknown();
struct DataType* typeBuilderGetUnknown();
struct StringDataType* typeBuilderGetStringType();

struct BasicDataTypes* typeBuilderBasicTypes();

#define TYPE_BUILDER_APPEND_SUB_TYPE(object, structureType, name, dataType) typeBuilderAppendSubType(object, #name, dataType, offsetof(structureType, name))

#endif