#include "test.h"

void testRef();
void testRefRelease();
void testBasicTypes();

int main(int argc, char* argv[]) {
    TEST_RUN(testRef);
    TEST_RUN(testRefRelease);
    TEST_RUN(testBasicTypes);

    return testRunSummary();
}