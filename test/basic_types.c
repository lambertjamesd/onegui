#include "../src/ref/basic_types.h"
#include "../src/ref/ref_privite.h"
#include "test.h"
#include "../src/types/string.h"

#include <math.h>

void testBasicTypes() {
    struct BasicDataTypes basicTypes;
    basicTypesInit(&basicTypes);

    for (unsigned i = 0; i < DataTypePrimitiveCount; ++i) {
        // create all the primitive data types
        TEST_ASSERT(refGetDataType(basicTypes.primitiveTypes[i]) == (struct DataType*)basicTypes.primitiveDataType);
        TEST_ASSERT(basicTypes.primitiveTypes[i]->type == i);
    }

    TEST_ASSERT(refGetDataType(basicTypes.objectDataType) == (struct DataType*)basicTypes.objectDataType); 
    TEST_ASSERT(basicTypes.objectDataType->type == DataTypeObject);
    TEST_ASSERT(basicTypes.objectDataType->byteSize == 8 + sizeof(void*));
    TEST_ASSERT(basicTypes.objectDataType->objectSubTypes->header.count == 4);
    TEST_ASSERT(basicTypes.objectDataType->objectSubTypes->elements[0].offset == 0);
    TEST_ASSERT(basicTypes.objectDataType->objectSubTypes->elements[0].type == basicTypes.primitiveTypes[DataTypeUInt16]);
    TEST_ASSERT(ostrEqualCStr(basicTypes.objectDataType->objectSubTypes->elements[0].name, "type"));
    TEST_ASSERT(basicTypes.objectDataType->objectSubTypes->elements[1].offset == 2);
    TEST_ASSERT(basicTypes.objectDataType->objectSubTypes->elements[1].type == basicTypes.primitiveTypes[DataTypeUInt16]);
    TEST_ASSERT(ostrEqualCStr(basicTypes.objectDataType->objectSubTypes->elements[1].name, "flags"));
    TEST_ASSERT(basicTypes.objectDataType->objectSubTypes->elements[2].offset == 4);
    TEST_ASSERT(basicTypes.objectDataType->objectSubTypes->elements[2].type == basicTypes.primitiveTypes[DataTypeUInt32]);
    TEST_ASSERT(ostrEqualCStr(basicTypes.objectDataType->objectSubTypes->elements[2].name, "byteSize"));
    TEST_ASSERT(basicTypes.objectDataType->objectSubTypes->elements[3].offset == 8);
    TEST_ASSERT(basicTypes.objectDataType->objectSubTypes->elements[3].type == (struct DataType*)basicTypes.pointerToObjectSubTypeArray);
    TEST_ASSERT(ostrEqualCStr(basicTypes.objectDataType->objectSubTypes->elements[3].name, "objectSubTypes"));

    TEST_ASSERT(refGetDataType(basicTypes.objectSubType) == (struct DataType*)basicTypes.objectDataType);
    TEST_ASSERT(basicTypes.objectSubType->type == DataTypeObject);
    TEST_ASSERT(basicTypes.objectSubType->objectSubTypes->header.count == 3);
    TEST_ASSERT(basicTypes.objectSubType->objectSubTypes->elements[0].offset == 0);
    TEST_ASSERT(basicTypes.objectSubType->objectSubTypes->elements[0].type == (struct DataType*)basicTypes.stringDataType);
    TEST_ASSERT(ostrEqualCStr(basicTypes.objectSubType->objectSubTypes->elements[0].name, "name"));
    TEST_ASSERT(basicTypes.objectSubType->objectSubTypes->elements[1].offset == sizeof(void*));
    TEST_ASSERT(basicTypes.objectSubType->objectSubTypes->elements[1].type == (struct DataType*)basicTypes.pointerToUnknownType);
    TEST_ASSERT(ostrEqualCStr(basicTypes.objectSubType->objectSubTypes->elements[1].name, "type"));
    TEST_ASSERT(basicTypes.objectSubType->objectSubTypes->elements[2].offset == sizeof(void*) * 2);
    TEST_ASSERT(basicTypes.objectSubType->objectSubTypes->elements[2].type == basicTypes.primitiveTypes[DataTypeUInt32]);
    TEST_ASSERT(ostrEqualCStr(basicTypes.objectSubType->objectSubTypes->elements[2].name, "offset"));

    TEST_ASSERT(refGetDataType(basicTypes.objectSubTypeArray) == (struct DataType*)basicTypes.dynamicArrayDataType);
    TEST_ASSERT(basicTypes.objectSubTypeArray->type == DataTypeDynamicArray);
    TEST_ASSERT(basicTypes.objectSubTypeArray->subType == (struct DataType*)basicTypes.objectSubType);

    TEST_ASSERT(refGetDataType(basicTypes.pointerToObjectSubTypeArray) == (struct DataType*)basicTypes.pointerDataType);
    TEST_ASSERT(basicTypes.pointerToObjectSubTypeArray->type == DataTypePointer);
    TEST_ASSERT(basicTypes.pointerToObjectSubTypeArray->subType == (struct DataType*)basicTypes.objectSubTypeArray);

    TEST_ASSERT(refGetDataType(basicTypes.primitiveDataType) == (struct DataType*)basicTypes.objectDataType);   
    TEST_ASSERT(basicTypes.primitiveDataType->type == DataTypeObject);
    TEST_ASSERT(basicTypes.primitiveDataType->byteSize == 4);
    TEST_ASSERT(basicTypes.primitiveDataType->objectSubTypes->header.count == 2);
    TEST_ASSERT(basicTypes.primitiveDataType->objectSubTypes->elements[0].offset == 0);
    TEST_ASSERT(basicTypes.primitiveDataType->objectSubTypes->elements[0].type == basicTypes.primitiveTypes[DataTypeUInt16]);
    TEST_ASSERT(ostrEqualCStr(basicTypes.primitiveDataType->objectSubTypes->elements[0].name, "type"));
    TEST_ASSERT(basicTypes.primitiveDataType->objectSubTypes->elements[1].offset == 2);
    TEST_ASSERT(basicTypes.primitiveDataType->objectSubTypes->elements[1].type == basicTypes.primitiveTypes[DataTypeUInt16]);
    TEST_ASSERT(ostrEqualCStr(basicTypes.primitiveDataType->objectSubTypes->elements[1].name, "flags"));

    TEST_ASSERT(refGetDataType(basicTypes.pointerDataType) == (struct DataType*)basicTypes.objectDataType);
    TEST_ASSERT(basicTypes.pointerDataType->type == DataTypeObject);
    TEST_ASSERT(basicTypes.pointerDataType->byteSize == 16);
    TEST_ASSERT(basicTypes.pointerDataType->objectSubTypes->header.count == 3);
    TEST_ASSERT(basicTypes.pointerDataType->objectSubTypes->elements[0].offset == 0);
    TEST_ASSERT(basicTypes.pointerDataType->objectSubTypes->elements[0].type == basicTypes.primitiveTypes[DataTypeUInt16]);
    TEST_ASSERT(ostrEqualCStr(basicTypes.pointerDataType->objectSubTypes->elements[0].name, "type"));
    TEST_ASSERT(basicTypes.pointerDataType->objectSubTypes->elements[1].offset == 2);
    TEST_ASSERT(basicTypes.pointerDataType->objectSubTypes->elements[1].type == basicTypes.primitiveTypes[DataTypeUInt16]);
    TEST_ASSERT(ostrEqualCStr(basicTypes.pointerDataType->objectSubTypes->elements[1].name, "flags"));
    TEST_ASSERT(basicTypes.pointerDataType->objectSubTypes->elements[2].offset == sizeof(void*));
    TEST_ASSERT(basicTypes.pointerDataType->objectSubTypes->elements[2].type == (struct DataType*)basicTypes.pointerToUnknownType);
    TEST_ASSERT(ostrEqualCStr(basicTypes.pointerDataType->objectSubTypes->elements[2].name, "subType"));

    TEST_ASSERT(refGetDataType(basicTypes.stringDataType) == (struct DataType*)basicTypes.primitiveDataType);
    TEST_ASSERT(basicTypes.stringDataType->type == DataTypeString);

    TEST_ASSERT(refGetDataType(basicTypes.unknownType) == (struct DataType*)basicTypes.primitiveDataType);
    TEST_ASSERT(basicTypes.unknownType->type == DataTypeUnknown);

    TEST_ASSERT(refGetDataType(basicTypes.pointerToUnknownType) == (struct DataType*)basicTypes.pointerDataType);
    TEST_ASSERT(basicTypes.pointerToUnknownType->type == DataTypePointer);
    TEST_ASSERT(basicTypes.pointerToUnknownType->subType == (struct DataType*)basicTypes.unknownType);

    TEST_ASSERT(refGetDataType(basicTypes.dynamicArrayDataType) == (struct DataType*)basicTypes.objectDataType);
    TEST_ASSERT(basicTypes.dynamicArrayDataType->type == DataTypeObject);
    TEST_ASSERT(basicTypes.dynamicArrayDataType->byteSize == 2 * sizeof(void*));
    TEST_ASSERT(basicTypes.dynamicArrayDataType->objectSubTypes->header.count == 3);
    TEST_ASSERT(basicTypes.dynamicArrayDataType->objectSubTypes->elements[0].offset == 0);
    TEST_ASSERT(basicTypes.dynamicArrayDataType->objectSubTypes->elements[0].type == basicTypes.primitiveTypes[DataTypeUInt16]);
    TEST_ASSERT(ostrEqualCStr(basicTypes.dynamicArrayDataType->objectSubTypes->elements[0].name, "type"));
    TEST_ASSERT(basicTypes.dynamicArrayDataType->objectSubTypes->elements[1].offset == 2);
    TEST_ASSERT(basicTypes.dynamicArrayDataType->objectSubTypes->elements[1].type == basicTypes.primitiveTypes[DataTypeUInt16]);
    TEST_ASSERT(ostrEqualCStr(basicTypes.dynamicArrayDataType->objectSubTypes->elements[1].name, "flags"));
    TEST_ASSERT(basicTypes.dynamicArrayDataType->objectSubTypes->elements[2].offset == sizeof(void*));
    TEST_ASSERT(basicTypes.dynamicArrayDataType->objectSubTypes->elements[2].type == (struct DataType*)basicTypes.pointerToUnknownType);
    TEST_ASSERT(ostrEqualCStr(basicTypes.dynamicArrayDataType->objectSubTypes->elements[2].name, "subType"));

    TEST_ASSERT(refGetDataType(basicTypes.fixedArrayDataType) == (struct DataType*)basicTypes.objectDataType);
    TEST_ASSERT(basicTypes.fixedArrayDataType->type == DataTypeObject);
    TEST_ASSERT(basicTypes.fixedArrayDataType->byteSize == (sizeof(void*) == 4 ? 12 : 16));
    TEST_ASSERT(basicTypes.fixedArrayDataType->objectSubTypes->header.count == 4);
    TEST_ASSERT(basicTypes.fixedArrayDataType->objectSubTypes->elements[0].offset == 0);
    TEST_ASSERT(basicTypes.fixedArrayDataType->objectSubTypes->elements[0].type == basicTypes.primitiveTypes[DataTypeUInt16]);
    TEST_ASSERT(ostrEqualCStr(basicTypes.fixedArrayDataType->objectSubTypes->elements[0].name, "type"));
    TEST_ASSERT(basicTypes.fixedArrayDataType->objectSubTypes->elements[1].offset == 2);
    TEST_ASSERT(basicTypes.fixedArrayDataType->objectSubTypes->elements[1].type == basicTypes.primitiveTypes[DataTypeUInt16]);
    TEST_ASSERT(ostrEqualCStr(basicTypes.fixedArrayDataType->objectSubTypes->elements[1].name, "flags"));
    TEST_ASSERT(basicTypes.fixedArrayDataType->objectSubTypes->elements[2].offset == 4);
    TEST_ASSERT(basicTypes.fixedArrayDataType->objectSubTypes->elements[2].type == basicTypes.primitiveTypes[DataTypeUInt32]);
    TEST_ASSERT(ostrEqualCStr(basicTypes.fixedArrayDataType->objectSubTypes->elements[2].name, "elementCount"));
    TEST_ASSERT(basicTypes.fixedArrayDataType->objectSubTypes->elements[3].offset == 8);
    TEST_ASSERT(basicTypes.fixedArrayDataType->objectSubTypes->elements[3].type == (struct DataType*)basicTypes.pointerToUnknownType);
    TEST_ASSERT(ostrEqualCStr(basicTypes.fixedArrayDataType->objectSubTypes->elements[3].name, "subType"));

    // add an extra reference before destroying basic types
    for (unsigned i = 0; i < DataTypePrimitiveCount; ++i) {
        refRetain(basicTypes.primitiveTypes[i]);
    }
    refRetain(basicTypes.objectDataType);
    refRetain(basicTypes.objectSubType);
    refRetain(basicTypes.objectSubTypeArray);
    refRetain(basicTypes.pointerToObjectSubTypeArray);
    refRetain(basicTypes.primitiveDataType);
    refRetain(basicTypes.pointerDataType);
    refRetain(basicTypes.stringDataType);
    refRetain(basicTypes.unknownType);
    refRetain(basicTypes.pointerToUnknownType);
    refRetain(basicTypes.dynamicArrayDataType);
    refRetain(basicTypes.fixedArrayDataType);

    basicTypesDestroy(&basicTypes);

    // verify destroy calls release the correct number of times
    for (unsigned i = 0; i < DataTypePrimitiveCount; ++i) {
        TEST_ASSERT(_refGetCount(basicTypes.primitiveTypes[i]) == 1);
        refRelease(basicTypes.primitiveTypes[i]);
    }
    TEST_ASSERT(_refGetCount(basicTypes.objectDataType) == 1);
    refRelease(basicTypes.objectDataType);
    TEST_ASSERT(_refGetCount(basicTypes.objectSubType) == 1);
    refRelease(basicTypes.objectSubType);
    TEST_ASSERT(_refGetCount(basicTypes.objectSubTypeArray) == 1);
    refRelease(basicTypes.objectSubTypeArray);
    TEST_ASSERT(_refGetCount(basicTypes.pointerToObjectSubTypeArray) == 1);
    refRelease(basicTypes.pointerToObjectSubTypeArray);
    TEST_ASSERT(_refGetCount(basicTypes.primitiveDataType) == 1);
    refRelease(basicTypes.primitiveDataType);
    TEST_ASSERT(_refGetCount(basicTypes.pointerDataType) == 1);
    refRelease(basicTypes.pointerDataType);
    TEST_ASSERT(_refGetCount(basicTypes.stringDataType) == 1);
    refRelease(basicTypes.stringDataType);
    TEST_ASSERT(_refGetCount(basicTypes.unknownType) == 1);
    refRelease(basicTypes.unknownType);
    TEST_ASSERT(_refGetCount(basicTypes.pointerToUnknownType) == 1);
    refRelease(basicTypes.pointerToUnknownType);
    TEST_ASSERT(_refGetCount(basicTypes.dynamicArrayDataType) == 1);
    refRelease(basicTypes.dynamicArrayDataType);
    TEST_ASSERT(_refGetCount(basicTypes.fixedArrayDataType) == 1);
    refRelease(basicTypes.fixedArrayDataType);
}

struct TestStructure {
    void* pointer;
    OString string;
};

void basicTypesObjectType() {
    struct BasicDataTypes basicTypes;
    basicTypesInit(&basicTypes);

    struct ObjectDataType* objectType = basicTypesNewObject(&basicTypes, sizeof(void*) * 2, 2);
    basicTypesAppendSubType(&basicTypes, objectType, "pointer", (struct DataType*)basicTypes.pointerToUnknownType, 0);
    basicTypesAppendSubType(&basicTypes, objectType, "string", (struct DataType*)basicTypes.stringDataType, sizeof(void*));

    void* pointerValue = refMalloc((struct DataType*)objectType);
    char* stringValue = refMallocString(basicTypes.stringDataType, 4, "test");

    struct TestStructure* test = refMalloc((struct DataType*)objectType);
    test->pointer = refRetain(pointerValue);
    test->string = refRetain(stringValue);

    TEST_ASSERT(_refGetCount(pointerValue) == 2);
    TEST_ASSERT(_refGetCount(stringValue) == 2);

    refRelease(test);

    // should automatically release any references
    TEST_ASSERT(_refGetCount(pointerValue) == 1);
    TEST_ASSERT(_refGetCount(stringValue) == 1);

    refRelease(pointerValue);
    refRelease(stringValue);

    refRelease(objectType);
    basicTypesDestroy(&basicTypes);
}

struct PointerArray {
    struct DynamicArrayHeader header;
    void* data[];
};

void basicTypesDynamicArrayType() {
    struct BasicDataTypes basicTypes;
    basicTypesInit(&basicTypes);

    struct DynamicArrayDataType* pointerArray = basicTypesNewVariableArray(&basicTypes, (struct DataType*)basicTypes.pointerToUnknownType);

    struct PointerArray* array = (struct PointerArray*)refMallocArray(pointerArray, 3);

    void* ptr = refMallocArray(pointerArray, 1);

    array->header.count = 2;
    array->data[0] = refRetain(ptr);

    // this reference should not be released 
    // since it isn't in the range for count even
    // if it is within the capacy to the array 
    array->data[2] = refRetain(ptr);

    TEST_ASSERT(_refGetCount(ptr) == 3);
    refRelease(array);
    TEST_ASSERT(_refGetCount(ptr) == 2);
    refRelease(ptr);
    refRelease(ptr);

    refRelease(pointerArray);

    basicTypesDestroy(&basicTypes);
}

void basicTypesFixedArrayType() {
    struct BasicDataTypes basicTypes;
    basicTypesInit(&basicTypes);

    struct FixedArrayDataType* pointerArray = basicTypesNewFixedArray(&basicTypes, (struct DataType*)basicTypes.pointerToUnknownType, 3);

    void* ptr = refMalloc((struct DataType*)pointerArray);
    void** array = refMalloc((struct DataType*)pointerArray);

    array[0] = refRetain(ptr);
    array[1] = refRetain(ptr);

    TEST_ASSERT(_refGetCount(ptr) == 3);
    refRelease(array);
    TEST_ASSERT(_refGetCount(ptr) == 1);

    refRelease(ptr);

    refRelease(pointerArray);
    basicTypesDestroy(&basicTypes);
}

struct StructWithWeakPointer {
    void* strongPointer;
    void* weakPointer;
};

void basicTypesWeakPointer() {
    struct BasicDataTypes basicTypes;
    basicTypesInit(&basicTypes);

    struct PointerDataType* weakUnknownPointer = basicTypesNewWeakPointerType(&basicTypes, (struct DataType*)basicTypes.unknownType);

    struct ObjectDataType* objectType = basicTypesNewObject(&basicTypes, sizeof(void*) * 2, 2);
    basicTypesAppendSubType(&basicTypes, objectType, "strongPointer", (struct DataType*)basicTypes.pointerToUnknownType, 0);
    basicTypesAppendSubType(&basicTypes, objectType, "weakPointer", (struct DataType*)weakUnknownPointer, sizeof(void*));

    void* ptr = refMalloc((struct DataType*)objectType);
    struct StructWithWeakPointer* structWithWeakPointer = refMalloc((struct DataType*)objectType);

    structWithWeakPointer->strongPointer = refRetain(ptr);
    structWithWeakPointer->weakPointer = ptr;

    TEST_ASSERT(_refGetCount(ptr) == 2);
    refRelease(structWithWeakPointer);
    TEST_ASSERT(_refGetCount(ptr) == 1);
    refRelease(ptr);

    refRelease(objectType);
    basicTypesDestroy(&basicTypes);
}

struct NestedStructure {
    void* ptr;
    struct TestStructure nested;
};

void basicTypesNestedStructure() {
    struct BasicDataTypes basicTypes;
    basicTypesInit(&basicTypes);

    struct ObjectDataType* nestedType = basicTypesNewObject(&basicTypes, sizeof(void*) * 2, 2);
    basicTypesAppendSubType(&basicTypes, nestedType, "pointer", (struct DataType*)basicTypes.pointerToUnknownType, 0);
    basicTypesAppendSubType(&basicTypes, nestedType, "string", (struct DataType*)basicTypes.stringDataType, sizeof(void*));

    struct ObjectDataType* objectType = basicTypesNewObject(&basicTypes, sizeof(void*) + sizeof(struct TestStructure), 2);
    basicTypesAppendSubType(&basicTypes, objectType, "ptr", (struct DataType*)basicTypes.pointerToUnknownType, 0);
    basicTypesAppendSubType(&basicTypes, objectType, "nested", (struct DataType*)nestedType, sizeof(void*));

    void* ptr = refMalloc((struct DataType*)nestedType);

    struct NestedStructure* nestedStructure = refMalloc((struct DataType*)objectType);

    OString str = refMallocString(basicTypes.stringDataType, 4, "test");

    nestedStructure->ptr = refRetain(ptr);
    nestedStructure->nested.pointer = refRetain(ptr);
    nestedStructure->nested.string = refRetain(str);

    TEST_ASSERT(_refGetCount(ptr) == 3);
    TEST_ASSERT(_refGetCount(str) == 2);
    refRelease(nestedStructure);
    TEST_ASSERT(_refGetCount(ptr) == 1);
    TEST_ASSERT(_refGetCount(str) == 1);
    refRelease(ptr);
    refRelease(str);

    refRelease(nestedType);
    refRelease(objectType);
    basicTypesDestroy(&basicTypes);

}