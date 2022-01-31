#ifndef __ONEGUI_EXPORTS_H__
#define __ONEGUI_EXPORTS_H__

#include <stdint.h>

#include "../data_structures/hash_table.h"
#include "../data_structures/ranged_binary_tree.h"

struct ObjectExportInformation {
    void* objectRef;
    OString moduleName;
    OString name;
};

struct SerializerState {
    struct RangedBinaryTree* addressToObjectInfo;
    struct HashTable* objectNameToObject;
};

struct NamedExport {
    OString name;
    void* exportValue;
};

struct NamedExportArray {
    struct DynamicArrayHeader header;
    struct NamedExport data[];
};

struct ModuleExports {
    struct NamedExport* typeExports;
    int32_t typeCount;
    struct NamedExport* valueExports;
    int32_t valueCount;
};

extern struct SerializerState* gSerializerState;
extern struct DynamicArrayDataType* gNamedExportArrayType;

void oneGuiExportsInit();

void oneGuiExportsAppend(OString moduleName, struct ModuleExports* exports);

void namedExportAppend(struct NamedExportArray* array, OString name, void* exportValue);

#endif