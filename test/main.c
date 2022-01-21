#include "test.h"

void testRef();
void testBasicTypes();
void basicTypesObjectType();
void basicTypesArrayType();

int main(int argc, char* argv[]) {
    TEST_RUN(testRef);
    TEST_RUN(testBasicTypes);
    TEST_RUN(basicTypesObjectType);
    TEST_RUN(basicTypesArrayType);

    return testRunSummary();
}