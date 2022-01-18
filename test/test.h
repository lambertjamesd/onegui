#ifndef __ONEGUI_TEST_TEST_H__
#define __ONEGUI_TEST_TEST_H__

#include <stdbool.h>

typedef void (*TestCallbak)();

extern char* gTestFailReason;
extern int gTestsRun;
extern int gTestsFailed;

void testAssert(bool assertion, char* assertionAsText);
void testRun(TestCallbak callback, char* testName);
int testRunSummary();

#define TEST_RUN(test) testRun(test, #test)
#define TEST_ASSERT(assertion) testAssert(assertion, #assertion)

#endif