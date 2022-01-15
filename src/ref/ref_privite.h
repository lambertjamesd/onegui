#ifndef __ONEGUI_REF_private_H__
#define __ONEGUI_REF_private_H__

#include "ref.h"

void* _ref_malloc_raw(size_t size);
struct DynamicArray* _ref_malloca_array_raw(size_t elementSize, unsigned capacity);
void* _ref_change_type(void* obj, struct DataType* dataType);

#endif