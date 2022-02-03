#ifndef __ONEGUI_STRING_H__
#define __ONEGUI_STRING_H__

#include <stdbool.h>
#include "types.h"

#include "../ref/ref.h"
#include "../types/type_builder.h"
#include <string.h>

#define OSTR_NEW_FROM_CSTR(cstr) refMallocString(typeBuilderGetStringType(), strlen(cstr), cstr)

OString ostrNewFromCStr(const char* cstr);
OString ostrNewFromLen(uint32_t len);
const char* ostrToCStr(ConstOString input, int* output);
int ostrLen(ConstOString ostr);
bool ostrEqual(ConstOString a, ConstOString b);
int ostrCompare(ConstOString a, ConstOString b);
bool ostrEqualCStr(ConstOString ostr, const char* cstr);

#endif