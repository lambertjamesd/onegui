#include "types.h"

static size_t gDataTypeSizes[] = {
    [DataTypeNull] = 0,
    [DataTypeInt8] = 1,
    [DataTypeUInt8] = 1,
    [DataTypeInt16] = 2,
    [DataTypeUInt16] = 2,
    [DataTypeInt32] = 4,
    [DataTypeUInt32] = 4,
    [DataTypeInt64] = 8,
    [DataTypeUInt64] = 8,
    [DataTypeFloat32] = 4,
    [DataTypeFloat64] = 8,

    [DataTypeWeakPointer] = sizeof(void*),
    [DataTypePointer] = sizeof(void*),
};

size_t dataTypeSize(struct DataType* type) {
    if (type->type < sizeof(gDataTypeSizes) / sizeof(*gDataTypeSizes)) {
        return gDataTypeSizes[type->type];
    }

    switch (type->type) {
        case DataTypeFixedArray:
        {
            struct FixedArrayDataType* asArrayType = (struct FixedArrayDataType*)type;
            return dataTypeSize(asArrayType->subType) * asArrayType->elementCount;
        }
        case DataTypeVariableArray:
            // minimum size, actual size may vary
            return sizeof(struct DynamicArrayHeader);
        case DataTypeObject:
        {
            struct ObjectDataType* asObjectType = (struct ObjectDataType*)type;
            return asObjectType->byteSize;
        }
    }

    return 0;
}