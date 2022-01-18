#include "test.h"
#include <setjmp.h>
#include <stdio.h>

int gTestsRun;
int gTestsFailed;
char* gTestFailReason;
jmp_buf* gTestJumpTarget;

void testAssert(bool assertion, char* assertionAsText) {
    if (!assertion) {
        gTestFailReason = assertionAsText;
        longjmp(*gTestJumpTarget, 1);
    }
}

void testRun(TestCallbak callback, char* testName) {
    ++gTestsRun;
    jmp_buf jumpBuffer;

    printf("Running test %s\n", testName);

    if (setjmp(jumpBuffer) != 0) {
        ++gTestsFailed;
        printf("Failed %s: Reason %s\n", testName, gTestFailReason);
        return;
    }

    gTestJumpTarget = &jumpBuffer;

    callback();

    printf("Passed %s\n", testName);
}

int testRunSummary() {
    printf("Tests run %d\n", gTestsRun);
    printf("Tests passed %d\n", gTestsFailed - gTestsRun);

    return gTestsFailed != 0;
}