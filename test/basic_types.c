#include "../src/ref/basic_types.h"
#include "../src/ref/ref_privite.h"
#include "test.h"
#include "../src/types/string.h"

void testBasicTypes() {
    struct BasicDataTypes basicTypes;
    basicTypesInit(&basicTypes);

    for (unsigned i = 0; i < DataTypePrimitiveCount; ++i) {
        // create all the primitive data types
        TEST_ASSERT(refGetDataType(basicTypes.primitiveTypes[i]) == (struct DataType*)basicTypes.primitiveDataType);
        TEST_ASSERT(_refGetCount(basicTypes.primitiveTypes[i]) == 1);
        TEST_ASSERT(basicTypes.primitiveTypes[i]->type == i);
    }

    TEST_ASSERT(refGetDataType(basicTypes.objectDataType) == (struct DataType*)basicTypes.objectDataType);   
    TEST_ASSERT(_refGetCount(basicTypes.objectDataType) == 7);
    TEST_ASSERT(basicTypes.objectDataType->type == DataTypeObject);
    TEST_ASSERT(basicTypes.objectDataType->byteSize == sizeof(struct ObjectDataType));
    TEST_ASSERT(basicTypes.objectDataType->objectSubTypes->header.count == 3);
    TEST_ASSERT(basicTypes.objectDataType->objectSubTypes->elements[0].offset == 0);
    TEST_ASSERT(basicTypes.objectDataType->objectSubTypes->elements[0].type == basicTypes.primitiveTypes[DataTypeUInt32]);
    TEST_ASSERT(ostrEqualCStr(basicTypes.objectDataType->objectSubTypes->elements[0].name, "type"));
    TEST_ASSERT(basicTypes.objectDataType->objectSubTypes->elements[1].offset == 4);
    TEST_ASSERT(basicTypes.objectDataType->objectSubTypes->elements[1].type == basicTypes.primitiveTypes[DataTypeUInt32]);
    TEST_ASSERT(ostrEqualCStr(basicTypes.objectDataType->objectSubTypes->elements[1].name, "byteSize"));
    TEST_ASSERT(basicTypes.objectDataType->objectSubTypes->elements[2].offset == 8);
    TEST_ASSERT(basicTypes.objectDataType->objectSubTypes->elements[2].type == (struct DataType*)basicTypes.objectSubTypeArray);
    TEST_ASSERT(ostrEqualCStr(basicTypes.objectDataType->objectSubTypes->elements[2].name, "objectSubTypes"));
}