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
    TEST_ASSERT(basicTypes.objectDataType->objectSubTypes->header.count == 3);
    TEST_ASSERT(basicTypes.objectDataType->objectSubTypes->elements[0].offset == 0);
    TEST_ASSERT(basicTypes.objectDataType->objectSubTypes->elements[0].type == basicTypes.primitiveTypes[DataTypeUInt32]);
    TEST_ASSERT(ostrEqualCStr(basicTypes.objectDataType->objectSubTypes->elements[0].name, "type"));
    TEST_ASSERT(basicTypes.objectDataType->objectSubTypes->elements[1].offset == 4);
    TEST_ASSERT(basicTypes.objectDataType->objectSubTypes->elements[1].type == basicTypes.primitiveTypes[DataTypeUInt32]);
    TEST_ASSERT(ostrEqualCStr(basicTypes.objectDataType->objectSubTypes->elements[1].name, "byteSize"));
    TEST_ASSERT(basicTypes.objectDataType->objectSubTypes->elements[2].offset == 8);
    TEST_ASSERT(basicTypes.objectDataType->objectSubTypes->elements[2].type == (struct DataType*)basicTypes.pointerToObjectSubTypeArray);
    TEST_ASSERT(ostrEqualCStr(basicTypes.objectDataType->objectSubTypes->elements[2].name, "objectSubTypes"));

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

    TEST_ASSERT(refGetDataType(basicTypes.objectSubTypeArray) == (struct DataType*)basicTypes.variableArrayDataType);
    TEST_ASSERT(basicTypes.objectSubTypeArray->type == DataTypeVariableArray);
    TEST_ASSERT(basicTypes.objectSubTypeArray->subType == (struct DataType*)basicTypes.objectSubType);

    TEST_ASSERT(refGetDataType(basicTypes.pointerToObjectSubTypeArray) == (struct DataType*)basicTypes.pointerDataType);
    TEST_ASSERT(basicTypes.pointerToObjectSubTypeArray->type == DataTypePointer);
    TEST_ASSERT(basicTypes.pointerToObjectSubTypeArray->subType == (struct DataType*)basicTypes.objectSubTypeArray);

    TEST_ASSERT(refGetDataType(basicTypes.primitiveDataType) == (struct DataType*)basicTypes.objectDataType);   
    TEST_ASSERT(basicTypes.primitiveDataType->type == DataTypeObject);
    TEST_ASSERT(basicTypes.primitiveDataType->byteSize == 4);
    TEST_ASSERT(basicTypes.primitiveDataType->objectSubTypes->header.count == 1);
    TEST_ASSERT(basicTypes.primitiveDataType->objectSubTypes->elements[0].offset == 0);
    TEST_ASSERT(basicTypes.primitiveDataType->objectSubTypes->elements[0].type == basicTypes.primitiveTypes[DataTypeUInt32]);
    TEST_ASSERT(ostrEqualCStr(basicTypes.primitiveDataType->objectSubTypes->elements[0].name, "type"));

    TEST_ASSERT(refGetDataType(basicTypes.pointerDataType) == (struct DataType*)basicTypes.objectDataType);
    TEST_ASSERT(basicTypes.pointerDataType->type == DataTypeObject);
    TEST_ASSERT(basicTypes.pointerDataType->byteSize == 16);
    TEST_ASSERT(basicTypes.pointerDataType->objectSubTypes->header.count == 2);
    TEST_ASSERT(basicTypes.pointerDataType->objectSubTypes->elements[0].offset == 0);
    TEST_ASSERT(basicTypes.pointerDataType->objectSubTypes->elements[0].type == basicTypes.primitiveTypes[DataTypeUInt32]);
    TEST_ASSERT(ostrEqualCStr(basicTypes.pointerDataType->objectSubTypes->elements[0].name, "type"));
    TEST_ASSERT(basicTypes.pointerDataType->objectSubTypes->elements[1].offset == sizeof(void*));
    TEST_ASSERT(basicTypes.pointerDataType->objectSubTypes->elements[1].type == (struct DataType*)basicTypes.pointerToUnknownType);
    TEST_ASSERT(ostrEqualCStr(basicTypes.pointerDataType->objectSubTypes->elements[1].name, "subType"));

    TEST_ASSERT(refGetDataType(basicTypes.stringDataType) == (struct DataType*)basicTypes.primitiveDataType);
    TEST_ASSERT(basicTypes.stringDataType->type == DataTypeString);

    TEST_ASSERT(refGetDataType(basicTypes.unknownType) == (struct DataType*)basicTypes.primitiveDataType);
    TEST_ASSERT(basicTypes.unknownType->type == DataTypeUnknown);

    TEST_ASSERT(refGetDataType(basicTypes.pointerToUnknownType) == (struct DataType*)basicTypes.pointerDataType);
    TEST_ASSERT(basicTypes.pointerToUnknownType->type == DataTypePointer);
    TEST_ASSERT(basicTypes.pointerToUnknownType->subType == (struct DataType*)basicTypes.unknownType);

    TEST_ASSERT(refGetDataType(basicTypes.variableArrayDataType) == (struct DataType*)basicTypes.objectDataType);
    TEST_ASSERT(basicTypes.variableArrayDataType->type == DataTypeObject);
    TEST_ASSERT(basicTypes.variableArrayDataType->byteSize == 2 * sizeof(void*));
    TEST_ASSERT(basicTypes.variableArrayDataType->objectSubTypes->header.count == 2);
    TEST_ASSERT(basicTypes.variableArrayDataType->objectSubTypes->elements[0].offset == 0);
    TEST_ASSERT(basicTypes.variableArrayDataType->objectSubTypes->elements[0].type == basicTypes.primitiveTypes[DataTypeUInt32]);
    TEST_ASSERT(ostrEqualCStr(basicTypes.variableArrayDataType->objectSubTypes->elements[0].name, "type"));
    TEST_ASSERT(basicTypes.variableArrayDataType->objectSubTypes->elements[1].offset == sizeof(void*));
    TEST_ASSERT(basicTypes.variableArrayDataType->objectSubTypes->elements[1].type == (struct DataType*)basicTypes.pointerToUnknownType);
    TEST_ASSERT(ostrEqualCStr(basicTypes.variableArrayDataType->objectSubTypes->elements[1].name, "subType"));

    TEST_ASSERT(refGetDataType(basicTypes.fixedArrayDataType) == (struct DataType*)basicTypes.objectDataType);
    TEST_ASSERT(basicTypes.fixedArrayDataType->type == DataTypeObject);
    TEST_ASSERT(basicTypes.fixedArrayDataType->byteSize == (sizeof(void*) == 4 ? 12 : 16));
    TEST_ASSERT(basicTypes.fixedArrayDataType->objectSubTypes->header.count == 3);
    TEST_ASSERT(basicTypes.fixedArrayDataType->objectSubTypes->elements[0].offset == 0);
    TEST_ASSERT(basicTypes.fixedArrayDataType->objectSubTypes->elements[0].type == basicTypes.primitiveTypes[DataTypeUInt32]);
    TEST_ASSERT(ostrEqualCStr(basicTypes.fixedArrayDataType->objectSubTypes->elements[0].name, "type"));
    TEST_ASSERT(basicTypes.fixedArrayDataType->objectSubTypes->elements[1].offset == 4);
    TEST_ASSERT(basicTypes.fixedArrayDataType->objectSubTypes->elements[1].type == basicTypes.primitiveTypes[DataTypeUInt32]);
    TEST_ASSERT(ostrEqualCStr(basicTypes.fixedArrayDataType->objectSubTypes->elements[1].name, "elementCount"));
    TEST_ASSERT(basicTypes.fixedArrayDataType->objectSubTypes->elements[2].offset == 8);
    TEST_ASSERT(basicTypes.fixedArrayDataType->objectSubTypes->elements[2].type == (struct DataType*)basicTypes.pointerToUnknownType);
    TEST_ASSERT(ostrEqualCStr(basicTypes.fixedArrayDataType->objectSubTypes->elements[2].name, "subType"));

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
    refRetain(basicTypes.variableArrayDataType);
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
    TEST_ASSERT(_refGetCount(basicTypes.variableArrayDataType) == 1);
    refRelease(basicTypes.variableArrayDataType);
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
    test->pointer = pointerValue;
    refRetain(pointerValue);
    test->string = stringValue;
    refRetain(stringValue);

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