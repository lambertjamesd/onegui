#ifndef __ONEGUI_REF_PRIVATE_H__
#define __ONEGUI_REF_PRIVATE_H__

#include "ref.h"

void* _refMallocRaw(size_t size);
int _refGetCount(void* obj);
void _refChangeType(void* obj, struct DataType* dataType);

#endif