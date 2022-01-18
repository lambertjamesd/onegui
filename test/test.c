#include "test.h"
#include <setjmp.h>
#include <stdio.h>

int gTestsRun;
int gTestsFailed;
char* gTestFailReason;
char* gTestFailFile;
int gTestFailLine;
jmp_buf* gTestJumpTarget;

void testAssert(bool assertion, char* assertionAsText, char* file, int line) {
    if (!assertion) {
        gTestFailReason = assertionAsText;
        gTestFailFile = file;
        gTestFailLine = line;
        longjmp(*gTestJumpTarget, 1);
    }
}

void testRun(TestCallbak callback, char* testName) {
    ++gTestsRun;
    jmp_buf jumpBuffer;

    printf("Running test %s\n", testName);

    if (setjmp(jumpBuffer) != 0) {
        ++gTestsFailed;
        printf("%s:%d: Failed %s\n    Reason %s\n", gTestFailFile, gTestFailLine, testName, gTestFailReason);
        return;
    }

    gTestJumpTarget = &jumpBuffer;

    callback();

    printf("Passed %s\n", testName);
}

int testRunSummary() {
    printf("Tests run %d passed %d\n", gTestsRun, gTestsRun - gTestsFailed);
    return gTestsFailed != 0;
}