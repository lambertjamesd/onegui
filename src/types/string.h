#ifndef __ONEGUI_STRING_H__
#define __ONEGUI_STRING_H__

#include <stdbool.h>

int ostrLen(const char* ostr);
bool ostrEqual(const char* a, const char* b);
bool ostrEqualCStr(const char* ostr, const char* cstr);

#endif