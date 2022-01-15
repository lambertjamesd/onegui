#ifndef __ONEGUI_REF_H__
#define __ONEGUI_REF_H__

#include "../types/types.h"

void* ref_malloc(struct DataType* dataType);
struct DynamicArray* ref_malloc_array(struct VariableArrayDataType* dataType, unsigned capacity);
OString ref_malloc_str(struct StringDataType* dataType, unsigned byteLength, char* dataSource);
void ref_retain(void* obj);
void ref_release(void* obj);

#endif