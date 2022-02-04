#ifndef __ONEGUI_MODULE_H__
#define __ONEGUI_MODULE_H__

#include "../data_structures/hash_table.h"

struct OGModule {
    struct HashTable *exports;
};

#endif