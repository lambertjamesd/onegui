#include "basic_types.h"
#include "ref_privite.h"

#include <string.h>

void basicTypesInit(struct BasicDataTypes* into) {
    // objectSubType, objectSubTypeArray, and objectDataType need to
    // exist before basicTypesNewObject can be used. So they are
    // bootstrapped manually ahead of time and finished later

    // ref type for struct ObjectSubType
    into->objectSubType = _refMallocRaw(sizeof(struct ObjectDataType));
    into->objectSubType->type = DataTypeObject;
    into->objectSubType->byteSize = sizeof(struct ObjectSubType);

    // ref type for struct ObjectSubTypeArray
    into->objectSubTypeArray = _refMallocRaw(sizeof(struct VariableArrayDataType));
    into->objectSubTypeArray->type = DataTypeVariableArray;
    into->objectSubTypeArray->subType = (struct DataType*)into->objectSubType;
    refRetain(into->objectSubType);
    
    // ref type for struct ObjectSubType
    into->objectDataType = _refMallocRaw(sizeof(struct ObjectDataType));
    into->objectDataType->type = DataTypeObject;
    into->objectDataType->byteSize = sizeof(struct ObjectDataType);

    // needed to create the string type
    into->primitiveDataType = basicTypesNewObject(into, sizeof(struct PrimitiveDataType), 1);
    
    // needed to call basicTypesAppendSubType
    into->stringDataType = refMalloc((struct DataType*)into->primitiveDataType);
    into->stringDataType->type = DataTypeString;

    for (unsigned i = 0; i < DataTypePrimitiveCount; ++i) {
        // create all the primitive data types
        into->primitiveTypes[i] = refMalloc((struct DataType*)into->primitiveDataType);
        into->primitiveTypes[i]->type = i;
    }

    // now that the string type is created basicTypesAppendSubType can be used to populate the primitiveDataType
    basicTypesAppendSubType(into, into->primitiveDataType, "type", into->primitiveTypes[DataTypeUInt32], 0);

    // build out pointer types starting with unknown type
    into->unknownType = refMalloc((struct DataType*)into->primitiveDataType);
    into->unknownType->type = DataTypeUnknown;

    into->pointerDataType = basicTypesNewObject(into, sizeof(struct PointerDataType), 2);
    into->pointerToUnknownType = basicTypesNewPointerType(into, into->unknownType);
    basicTypesAppendSubType(into, into->pointerDataType, "type", into->primitiveTypes[DataTypeUInt32], 0);
    basicTypesAppendSubType(into, into->pointerDataType, "subType", (struct DataType*)into->pointerToUnknownType, offsetof(struct PointerDataType, subType));

    into->variableArrayDataType = basicTypesNewObject(into, sizeof(struct VariableArrayDataType), 2);
    basicTypesAppendSubType(into, into->variableArrayDataType, "type", into->primitiveTypes[DataTypeUInt32], 0);
    basicTypesAppendSubType(into, into->variableArrayDataType, "subType", (struct DataType*)into->pointerToUnknownType, offsetof(struct VariableArrayDataType, subType));

    into->fixedArrayDataType = basicTypesNewObject(into, sizeof(struct FixedArrayDataType), 3);
    basicTypesAppendSubType(into, into->fixedArrayDataType, "type", into->primitiveTypes[DataTypeUInt32], 0);
    basicTypesAppendSubType(into, into->fixedArrayDataType, "elementCount", into->primitiveTypes[DataTypeUInt32], offsetof(struct FixedArrayDataType, elementCount));
    basicTypesAppendSubType(into, into->fixedArrayDataType, "subType", (struct DataType*)into->pointerToUnknownType, offsetof(struct FixedArrayDataType, subType));

    // everything needed to call the helper functions are finished, now 
    // go back and popluate the objects that were manually created

    _refChangeType(into->objectSubType, (struct DataType*)into->objectDataType);
    into->objectSubType->objectSubTypes = (struct ObjectSubTypeArray*)refMallocArray(into->objectSubTypeArray, 3);
    basicTypesAppendSubType(into, into->objectSubType, "name", (struct DataType*)into->stringDataType, 0);
    basicTypesAppendSubType(into, into->objectSubType, "type", (struct DataType*)into->pointerToUnknownType, offsetof(struct ObjectSubType, type));
    basicTypesAppendSubType(into, into->objectSubType, "offset", into->primitiveTypes[DataTypeUInt32], offsetof(struct ObjectSubType, offset));

    _refChangeType(into->objectSubTypeArray, (struct DataType*)into->variableArrayDataType);
    _refChangeType(into->objectDataType, (struct DataType*)into->objectDataType);
    into->objectDataType->objectSubTypes = (struct ObjectSubTypeArray*)refMallocArray(into->objectSubTypeArray, 3);
    basicTypesAppendSubType(into, into->objectDataType, "type", into->primitiveTypes[DataTypeUInt32], 0);
    basicTypesAppendSubType(into, into->objectDataType, "byteSize", into->primitiveTypes[DataTypeUInt32], offsetof(struct ObjectDataType, byteSize));
    basicTypesAppendSubType(into, into->objectDataType, "objectSubTypes", (struct DataType*)into->objectSubTypeArray, offsetof(struct ObjectDataType, objectSubTypes));

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
    _basicTypesDestroyObjectDataType(basicTypes->variableArrayDataType);
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

    onto->objectSubTypes->header.count++;
}

struct PointerDataType* basicTypesNewPointerType(struct BasicDataTypes* basicTypes, struct DataType* pointTo) {
    struct PointerDataType* result = refMalloc((struct DataType*)basicTypes->pointerDataType);
    result->type = DataTypePointer;
    result->subType = pointTo;
    refRetain(pointTo);
    return result;
}

struct PointerDataType* basicTypesNewWeakPointerType(struct BasicDataTypes* basicTypes, struct DataType* pointTo) {
    struct PointerDataType* result = refMalloc((struct DataType*)basicTypes->pointerDataType);
    result->type = DataTypeWeakPointer;
    result->subType = pointTo;
    refRetain(pointTo);
    return result;
}

struct VariableArrayDataType* basicTypesNewVariableArray(struct BasicDataTypes* basicTypes, struct DataType* elementType) {
    struct VariableArrayDataType* result = refMalloc((struct DataType*)basicTypes->variableArrayDataType);
    result->type = DataTypeVariableArray;
    result->subType = elementType;
    refRetain(elementType);
    return result;
}

struct FixedArrayDataType* basicTypesNewFixedArray(struct BasicDataTypes* basicTypes, struct DataType* elementType, unsigned size) {
    struct FixedArrayDataType* result = refMalloc((struct DataType*)basicTypes->fixedArrayDataType);
    result->type = DataTypeVariableArray;
    result->subType = elementType;
    refRetain(elementType);
    result->elementCount = size;
    return result;
}