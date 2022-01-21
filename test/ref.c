
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