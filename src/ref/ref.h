#ifndef __ONEGUI_REF_H__
#define __ONEGUI_REF_H__

#include "../types/types.h"

void* refMalloc(struct DataType* dataType);
struct DynamicArray* refMallocArray(struct VariableArrayDataType* dataType, unsigned capacity);
OString refMallocString(struct StringDataType* dataType, unsigned byteLength, char* dataSource);
void refRetain(void* obj);
void refRelease(void* obj);
struct DataType* refGetDataType(void* obj);

#endif