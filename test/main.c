#include "test.h"

void testRef();
void testBasicTypes();
void basicTypesObjectType();
void basicTypesDynamicArrayType();
void basicTypesFixedArrayType();
void basicTypesWeakPointer();
void basicTypesNestedStructure();

int main(int argc, char* argv[]) {
    TEST_RUN(testRef);
    TEST_RUN(testBasicTypes);
    TEST_RUN(basicTypesObjectType);
    TEST_RUN(basicTypesDynamicArrayType);
    TEST_RUN(basicTypesFixedArrayType);
    TEST_RUN(basicTypesWeakPointer);
    TEST_RUN(basicTypesNestedStructure);

    return testRunSummary();
}