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
    into->objectSubType->byteSize = sizeof(struct ObjectDataType);

    // ref type for struct ObjectSubTypeArray
    into->objectSubTypeArray = _refMallocRaw(sizeof(struct VariableArrayDataType));
    into->objectSubTypeArray->type = DataTypeVariableArray;
    into->objectSubTypeArray->subType = into->objectSubType;
    refRetain(into->objectSubType);
    
    // ref type for struct ObjectSubType
    into->objectDataType = _refMallocRaw(sizeof(struct ObjectDataType));
    into->objectDataType->type = DataTypeObject;
    into->objectDataType->byteSize = sizeof(struct ObjectDataType);

    // needed to create the string type
    into->primitiveDataType = basicTypesNewObject(into, sizeof(struct PrimitiveDataType), 1);
    
    // needed to call basicTypesAppendSubType
    into->stringDataType = refMalloc(into->primitiveDataType);
    into->stringDataType->type = DataTypeString;

    for (unsigned i = 0; i < DataTypePrimitiveCount; ++i) {
        // create all the primitive data types
        into->primitiveTypes[i] = refMalloc(into->primitiveDataType);
        into->primitiveTypes[i]->type = i;
    }

    // now that the string type is created basicTypesAppendSubType can be used to populate the primitiveDataType
    basicTypesAppendSubType(into, into->primitiveDataType, "type", into->primitiveTypes[DataTypeUInt32], 0);

    // build out pointer types starting with unknown type
    into->unknownType = refMalloc(into->primitiveDataType);
    into->unknownType->type = DataTypeUnknown;

    into->pointerDataType = basicTypesNewObject(into, sizeof(struct PointerDataType), 2);
    into->pointerToUnknownType = basicTypesNewPointerType(into, into->unknownType);
    basicTypesAppendSubType(into, into->pointerDataType, "type", into->primitiveTypes[DataTypeUInt32], 0);
    basicTypesAppendSubType(into, into->pointerDataType, "subType", into->pointerToUnknownType, offsetof(struct PointerDataType, subType));

    into->variableArrayDataType = basicTypesNewObject(into, sizeof(struct VariableArrayDataType), 2);
    basicTypesAppendSubType(into, into->variableArrayDataType, "type", into->primitiveTypes[DataTypeUInt32], 0);
    basicTypesAppendSubType(into, into->variableArrayDataType, "subType", into->pointerToUnknownType, offsetof(struct VariableArrayDataType, subType));

    into->variableArrayDataType = basicTypesNewObject(into, sizeof(struct FixedArrayDataType), 3);
    basicTypesAppendSubType(into, into->variableArrayDataType, "type", into->primitiveTypes[DataTypeUInt32], 0);
    basicTypesAppendSubType(into, into->variableArrayDataType, "subType", into->pointerToUnknownType, offsetof(struct FixedArrayDataType, subType));
    basicTypesAppendSubType(into, into->variableArrayDataType, "elementCount", into->primitiveTypes[DataTypeUInt32], offsetof(struct FixedArrayDataType, elementCount));

    // everything needed to call the helper functions are finished, now 
    // go back and popluate the objects that were manually created

    _refChangeType(into->objectSubType, into->objectDataType);
    into->objectSubType->objectSubTypes = refMallocArray(into->objectSubTypeArray, 3);
    basicTypesAppendSubType(into, into->objectSubType, "name", into->stringDataType, 0);
    basicTypesAppendSubType(into, into->objectSubType, "type", into->pointerToUnknownType, offsetof(struct ObjectSubType, type));
    basicTypesAppendSubType(into, into->objectSubType, "offset", into->primitiveTypes[DataTypeUInt32], offsetof(struct ObjectSubType, offset));

    _refChangeType(into->objectSubTypeArray, into->variableArrayDataType);
    _refChangeType(into->objectDataType, into->objectDataType);
    into->objectDataType->objectSubTypes = refMallocArray(into->objectSubTypeArray, 3);
    basicTypesAppendSubType(into, into->objectDataType, "type", into->primitiveTypes[DataTypeUInt32], 0);
    basicTypesAppendSubType(into, into->objectDataType, "byteSize", into->primitiveTypes[DataTypeUInt32], offsetof(struct ObjectDataType, byteSize));
    basicTypesAppendSubType(into, into->objectDataType, "objectSubTypes", into->objectSubTypeArray, offsetof(struct ObjectDataType, objectSubTypes));

    _refChangeType(into->stringDataType, into->primitiveDataType);
}

struct DataType* basicTypesGetPrimitive(struct BasicDataTypes* basicTypes, enum DataTypeType type) {
    if (type >= DataTypePrimitiveCount) {
        return 0;
    }

    return basicTypes->primitiveTypes[type];
}

struct ObjectDataType* basicTypesNewObject(struct BasicDataTypes* basicTypes, size_t size, unsigned elementCount) {
    struct ObjectDataType* result = refMalloc(basicTypes->objectDataType);
    result->type = DataTypeObject;
    result->byteSize = size;
    result->objectSubTypes = refMallocArray(basicTypes->objectSubTypeArray, elementCount);

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
    struct PointerDataType* result = refMalloc(basicTypes->pointerDataType);
    result->type = DataTypePointer;
    result->subType = pointTo;
    refRetain(pointTo);
}

struct PointerDataType* basicTypesNewWeakPointerType(struct BasicDataTypes* basicTypes, struct DataType* pointTo) {
    struct PointerDataType* result = refMalloc(basicTypes->pointerDataType);
    result->type = DataTypeWeakPointer;
    result->subType = pointTo;
    refRetain(pointTo);
}

struct VariableArrayDataType* basicTypesNewVariableArray(struct BasicDataTypes* basicTypes, struct DataType* elementType) {
    struct VariableArrayDataType* result = refMalloc(basicTypes->variableArrayDataType);
    result->type = DataTypeVariableArray;
    result->subType = elementType;
    refRetain(elementType);
    return result;
}

struct FixedArrayDataType* basicTypesNewFixedArray(struct BasicDataTypes* basicTypes, struct DataType* elementType, unsigned size) {
    struct FixedArrayDataType* result = refMalloc(basicTypes->fixedArrayDataType);
    result->type = DataTypeVariableArray;
    result->subType = elementType;
    refRetain(elementType);
    result->elementCount = size;
    return result;
}