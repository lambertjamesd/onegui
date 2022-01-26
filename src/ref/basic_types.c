#include "basic_types.h"
#include "ref_privite.h"

#include <string.h>

void basicTypesInit(struct BasicDataTypes* into) {
    // objectSubType, objectSubTypeArray, and objectDataType need to
    // exist before basicTypesNewObject can be used. So they are
    // bootstrapped manually ahead of time and finished later

    // ref type for struct ObjectSubType
    into->objectSubType = _refMallocRaw(sizeof(struct ObjectDataType));
    // flags are populated later when appending sub types
    into->objectSubType->flags = 0;
    into->objectSubType->type = DataTypeObject;
    into->objectSubType->byteSize = sizeof(struct ObjectSubType);

    // ref type for struct ObjectSubTypeArray
    into->objectSubTypeArray = _refMallocRaw(sizeof(struct DynamicArrayDataType));
    into->objectSubTypeArray->flags = DataTypeFlagsHasStrongRef;
    into->objectSubTypeArray->type = DataTypeDynamicArray;
    into->objectSubTypeArray->subType = (struct DataType*)into->objectSubType;
    refRetain(into->objectSubType);

    into->pointerToObjectSubTypeArray = _refMallocRaw(sizeof(struct PointerDataType));
    into->pointerToObjectSubTypeArray->type = DataTypePointer;
    into->pointerToObjectSubTypeArray->flags = DataTypeFlagsHasStrongRef;
    into->pointerToObjectSubTypeArray->subType = (struct DataType*)into->objectSubTypeArray;
    refRetain(into->pointerToObjectSubTypeArray->subType);

    // ref type for struct ObjectSubType
    into->objectDataType = _refMallocRaw(sizeof(struct ObjectDataType));
    into->objectDataType->type = DataTypeObject;
    // flags are populated later when appending sub types
    into->objectDataType->flags = 0;
    into->objectDataType->byteSize = sizeof(struct ObjectDataType);

    // needed to create the string type
    into->primitiveDataType = basicTypesNewObject(into, sizeof(struct PrimitiveDataType), 2);
    
    // needed to call basicTypesAppendSubType
    into->stringDataType = refMalloc((struct DataType*)into->primitiveDataType);
    into->stringDataType->flags = DataTypeFlagsHasStrongRef;
    into->stringDataType->type = DataTypeString;

    for (unsigned i = 0; i < DataTypePrimitiveCount; ++i) {
        // create all the primitive data types
        into->primitiveTypes[i] = refMalloc((struct DataType*)into->primitiveDataType);
        into->primitiveTypes[i]->type = i;
    }

    // now that the string type is created basicTypesAppendSubType can be used to populate the primitiveDataType
    basicTypesAppendSubType(into, into->primitiveDataType, "type", into->primitiveTypes[DataTypeUInt16], 0);
    basicTypesAppendSubType(into, into->primitiveDataType, "flags", into->primitiveTypes[DataTypeUInt16], offsetof(struct PrimitiveDataType, flags));

    // build out pointer types starting with unknown type
    into->unknownType = refMalloc((struct DataType*)into->primitiveDataType);
    into->unknownType->type = DataTypeUnknown;

    into->pointerDataType = basicTypesNewObject(into, sizeof(struct PointerDataType), 3);
    into->pointerToUnknownType = basicTypesNewPointerType(into, into->unknownType);
    basicTypesAppendSubType(into, into->pointerDataType, "type", into->primitiveTypes[DataTypeUInt16], 0);
    basicTypesAppendSubType(into, into->pointerDataType, "flags", into->primitiveTypes[DataTypeUInt16], offsetof(struct PointerDataType, flags));
    basicTypesAppendSubType(into, into->pointerDataType, "subType", (struct DataType*)into->pointerToUnknownType, offsetof(struct PointerDataType, subType));
    _refChangeType(into->pointerToObjectSubTypeArray, (struct DataType*)into->pointerDataType);

    into->dynamicArrayDataType = basicTypesNewObject(into, sizeof(struct DynamicArrayDataType), 3);
    basicTypesAppendSubType(into, into->dynamicArrayDataType, "type", into->primitiveTypes[DataTypeUInt16], 0);
    basicTypesAppendSubType(into, into->dynamicArrayDataType, "flags", into->primitiveTypes[DataTypeUInt16], offsetof(struct DynamicArrayDataType, flags));
    basicTypesAppendSubType(into, into->dynamicArrayDataType, "subType", (struct DataType*)into->pointerToUnknownType, offsetof(struct DynamicArrayDataType, subType));

    into->fixedArrayDataType = basicTypesNewObject(into, sizeof(struct FixedArrayDataType), 4);
    basicTypesAppendSubType(into, into->fixedArrayDataType, "type", into->primitiveTypes[DataTypeUInt16], 0);
    basicTypesAppendSubType(into, into->fixedArrayDataType, "flags", into->primitiveTypes[DataTypeUInt16], offsetof(struct FixedArrayDataType, flags));
    basicTypesAppendSubType(into, into->fixedArrayDataType, "elementCount", into->primitiveTypes[DataTypeUInt32], offsetof(struct FixedArrayDataType, elementCount));
    basicTypesAppendSubType(into, into->fixedArrayDataType, "subType", (struct DataType*)into->pointerToUnknownType, offsetof(struct FixedArrayDataType, subType));

    // everything needed to call the helper functions are finished, now 
    // go back and popluate the objects that were manually created

    _refChangeType(into->objectSubType, (struct DataType*)into->objectDataType);
    into->objectSubType->objectSubTypes = (struct ObjectSubTypeArray*)refMallocArray(into->objectSubTypeArray, 3);
    basicTypesAppendSubType(into, into->objectSubType, "name", (struct DataType*)into->stringDataType, 0);
    basicTypesAppendSubType(into, into->objectSubType, "type", (struct DataType*)into->pointerToUnknownType, offsetof(struct ObjectSubType, type));
    basicTypesAppendSubType(into, into->objectSubType, "offset", into->primitiveTypes[DataTypeUInt32], offsetof(struct ObjectSubType, offset));

    _refChangeType(into->objectSubTypeArray, (struct DataType*)into->dynamicArrayDataType);
    _refChangeType(into->objectDataType, (struct DataType*)into->objectDataType);
    into->objectDataType->objectSubTypes = (struct ObjectSubTypeArray*)refMallocArray(into->objectSubTypeArray, 4);
    basicTypesAppendSubType(into, into->objectDataType, "type", into->primitiveTypes[DataTypeUInt16], 0);
    basicTypesAppendSubType(into, into->objectDataType, "flags", into->primitiveTypes[DataTypeUInt16], offsetof(struct ObjectDataType, flags));
    basicTypesAppendSubType(into, into->objectDataType, "byteSize", into->primitiveTypes[DataTypeUInt32], offsetof(struct ObjectDataType, byteSize));
    basicTypesAppendSubType(into, into->objectDataType, "objectSubTypes", (struct DataType*)into->pointerToObjectSubTypeArray, offsetof(struct ObjectDataType, objectSubTypes));

    _refChangeType(into->stringDataType, (struct DataType*)into->primitiveDataType);
}

void _basicTypesDestroyObjectDataType(struct ObjectDataType* objectDataType) {
    struct ObjectSubType* end = objectDataType->objectSubTypes->elements + objectDataType->objectSubTypes->header.count;
    for (struct ObjectSubType* curr = objectDataType->objectSubTypes->elements; curr < end; ++curr) {
        refRelease(curr->type);
        curr->type = NULL;
        refRelease(curr->name);
        curr->name = NULL;
    }
    _refChangeType(objectDataType, NULL);
    refRelease(objectDataType->objectSubTypes);
    refRelease(objectDataType);
}

void basicTypesDestroy(struct BasicDataTypes* basicTypes) {
    for (unsigned i = 0; i < DataTypePrimitiveCount; ++i) {
        _refChangeType(basicTypes->primitiveTypes[i], NULL);
        refRelease(basicTypes->primitiveTypes[i]);
    }

    _basicTypesDestroyObjectDataType(basicTypes->objectDataType);
    _basicTypesDestroyObjectDataType(basicTypes->objectSubType);

    _refChangeType(basicTypes->objectSubTypeArray, NULL);
    refRelease(basicTypes->objectSubTypeArray->subType);
    basicTypes->objectSubTypeArray->subType = NULL;
    refRelease(basicTypes->objectSubTypeArray);

    _refChangeType(basicTypes->pointerToObjectSubTypeArray, NULL);
    refRelease(basicTypes->pointerToObjectSubTypeArray->subType);
    basicTypes->pointerToObjectSubTypeArray->subType = NULL;
    refRelease(basicTypes->pointerToObjectSubTypeArray);

    _basicTypesDestroyObjectDataType(basicTypes->primitiveDataType);
    _basicTypesDestroyObjectDataType(basicTypes->pointerDataType);

    _refChangeType(basicTypes->stringDataType, NULL);
    refRelease(basicTypes->stringDataType);

    _refChangeType(basicTypes->unknownType, NULL);
    refRelease(basicTypes->unknownType);

    _refChangeType(basicTypes->pointerToUnknownType, NULL);
    refRelease(basicTypes->pointerToUnknownType->subType);
    basicTypes->pointerToUnknownType->subType = NULL;
    refRelease(basicTypes->pointerToUnknownType);

    _basicTypesDestroyObjectDataType(basicTypes->dynamicArrayDataType);
    _basicTypesDestroyObjectDataType(basicTypes->fixedArrayDataType);
}

struct DataType* basicTypesGetPrimitive(struct BasicDataTypes* basicTypes, enum DataTypeType type) {
    if (type >= DataTypePrimitiveCount) {
        return 0;
    }

    return basicTypes->primitiveTypes[type];
}

struct ObjectDataType* basicTypesNewObject(struct BasicDataTypes* basicTypes, size_t size, unsigned elementCount) {
    struct ObjectDataType* result = refMalloc((struct DataType*)basicTypes->objectDataType);
    result->type = DataTypeObject;
    result->flags = 0;
    result->byteSize = size;
    result->objectSubTypes = (struct ObjectSubTypeArray*)refMallocArray(basicTypes->objectSubTypeArray, elementCount);

    return result;
}

void basicTypesAppendSubType(struct BasicDataTypes* basicTypes, struct ObjectDataType* onto, char* cStrName, struct DataType* subType, unsigned offset) {
    unsigned curr = onto->objectSubTypes->header.count;

    if (curr >= onto->objectSubTypes->header.capacity) {
        return;
    }

    struct ObjectSubType* target = &onto->objectSubTypes->elements[curr];
    target->name = refMallocString(basicTypes->stringDataType, strlen(cStrName), cStrName);
    target->type = subType;
    refRetain(subType);
    target->offset = offset;

    onto->flags |= subType->flags & (DataTypeFlagsHasStrongRef | DataTypeFlagsHasWeakRef);

    onto->objectSubTypes->header.count++;
}

struct PointerDataType* basicTypesNewPointerType(struct BasicDataTypes* basicTypes, struct DataType* pointTo) {
    struct PointerDataType* result = refMalloc((struct DataType*)basicTypes->pointerDataType);
    result->type = DataTypePointer;
    result->flags = DataTypeFlagsHasStrongRef;
    result->subType = pointTo;
    refRetain(pointTo);
    return result;
}

struct PointerDataType* basicTypesNewWeakPointerType(struct BasicDataTypes* basicTypes, struct DataType* pointTo) {
    struct PointerDataType* result = refMalloc((struct DataType*)basicTypes->pointerDataType);
    result->type = DataTypeWeakPointer;
    result->flags = DataTypeFlagsHasWeakRef;
    result->subType = pointTo;
    refRetain(pointTo);
    return result;
}

struct DynamicArrayDataType* basicTypesNewVariableArray(struct BasicDataTypes* basicTypes, struct DataType* elementType) {
    struct DynamicArrayDataType* result = refMalloc((struct DataType*)basicTypes->dynamicArrayDataType);
    result->type = DataTypeDynamicArray;
    result->flags = elementType->flags & (DataTypeFlagsHasStrongRef | DataTypeFlagsHasWeakRef);
    result->subType = elementType;
    refRetain(elementType);
    return result;
}

struct FixedArrayDataType* basicTypesNewFixedArray(struct BasicDataTypes* basicTypes, struct DataType* elementType, unsigned size) {
    struct FixedArrayDataType* result = refMalloc((struct DataType*)basicTypes->fixedArrayDataType);
    result->type = DataTypeFixedArray;
    result->flags = elementType->flags & (DataTypeFlagsHasStrongRef | DataTypeFlagsHasWeakRef);
    result->subType = elementType;
    refRetain(elementType);
    result->elementCount = size;
    return result;
}