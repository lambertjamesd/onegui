#ifndef __ONEGUI_MODULE_H__
#define __ONEGUI_MODULE_H__

#include "../data_structures/hash_table.h"

struct NamedExport {
    OString name;
    void* exportValue;
};

struct NamedExportArray {
    struct DynamicArrayHeader header;
    struct NamedExport data[];
};

struct ModuleExports {
    struct NamedExportArray* types;
    struct NamedExportArray* values;
};

struct OGModule {
    OString name;
    struct HashTable *exports;
};

struct OGModule* ogModuleNew(OString name);
void ogModuleAppend(struct OGModule* module, OString name, void* ref);
void ogModuleAppendExports(struct OGModule* module, struct ModuleExports* exports);

#endif