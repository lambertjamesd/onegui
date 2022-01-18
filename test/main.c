#include "test.h"

void testHelloWorld() {
    TEST_ASSERT(1 == 1);
}

int main(int argc, char* argv[]) {

    TEST_RUN(testHelloWorld);

    return testRunSummary();
}