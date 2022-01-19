
#include "test.h"
#include "../src/ref/ref.h"
#include "../src/ref/ref_privite.h"
#include "../src/ref/basic_types.h"

void testRef() {
    void* ptr = _refMallocRaw(16);
    TEST_ASSERT(_refGetCount(ptr) == 1);

    refRetain(ptr);
    TEST_ASSERT(_refGetCount(ptr) == 2);

    refRelease(ptr);
    TEST_ASSERT(_refGetCount(ptr) == 1);

    refRelease(ptr);
}

void testRefRelease() {
    struct PointerDataType* pointerType = _refMallocRaw(sizeof(struct PointerDataType));

    pointerType->type = DataTypePointer;
    pointerType->subType = 0;

    int** ptrType = refMalloc((struct DataType*)pointerType);
    int* pointTo = _refMallocRaw(sizeof(int));
    *ptrType = pointTo;
    refRetain(pointTo);

    TEST_ASSERT(_refGetCount(pointTo) == 2);

    // verify that when the pointer type is released it 
    // decrements the count on the object it was pointing to
    refRelease(ptrType);
    TEST_ASSERT(_refGetCount(pointTo) == 1);

    refRelease(pointTo);

    refRelease(pointerType);
}