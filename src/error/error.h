#ifndef __ONEGUIE_ERROR_H__
#define __ONEGUIE_ERROR_H__

enum OGError {
    OGErrorNone,
    OGErrorUnknown,
};

void ogReportError(const char* cstrMessage);
void ogReportStack(const char* file, int lineNumber);

const char* ogGetLastError();

#endif