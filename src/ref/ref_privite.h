#ifndef __ONEGUI_REF_private_H__
#define __ONEGUI_REF_private_H__

#include "ref.h"

void* _refMallocRaw(size_t size);
void _refChangeType(void* obj, struct DataType* dataType);

#endif