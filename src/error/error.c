#include "error.h"

#include <string.h>

#define MAX_ERROR_LENGTH    256

char _ogLastError[MAX_ERROR_LENGTH];

void ogReportError(const char* cstrMessage) {
    strncpy(_ogLastError, cstrMessage, MAX_ERROR_LENGTH);
}

void ogReportStack(const char* file, int lineNumber) {
    // TODO
}

const char* ogGetLastError() {
    return _ogLastError;
}