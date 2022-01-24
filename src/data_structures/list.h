#ifndef __ONEGUI_LIST_H__
#define __ONEGUI_LIST_H__

#include "../types/types.h"

struct OGList {
    struct DynamicArrayDataType* arrayType;
    struct DynamicArray* array;
    unsigned elementSize;
};

void ogListInit();

struct OGList* ogListNew();
void* ogListGet(struct OGList* list, unsigned index);
void ogListSet(struct OGList* list, unsigned index, void* src);

#endif