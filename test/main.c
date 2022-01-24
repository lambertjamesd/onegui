#include "test.h"

#include "../src/main/onegui.h"

void testRef();
void testBasicTypes();
void basicTypesObjectType();
void basicTypesDynamicArrayType();
void basicTypesFixedArrayType();
void basicTypesWeakPointer();
void basicTypesNestedStructure();
void testHashTable();
void testHashTableKeyReference();
void testRangedBinaryTree();

int main(int argc, char* argv[]) {
    oneGuiInit();

    TEST_RUN(testRef);
    TEST_RUN(testBasicTypes);
    TEST_RUN(basicTypesObjectType);
    TEST_RUN(basicTypesDynamicArrayType);
    TEST_RUN(basicTypesFixedArrayType);
    TEST_RUN(basicTypesWeakPointer);
    TEST_RUN(basicTypesNestedStructure);
    TEST_RUN(testHashTable);
    TEST_RUN(testHashTableKeyReference);
    TEST_RUN(testRangedBinaryTree);

    return testRunSummary();
}