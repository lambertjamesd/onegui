#ifndef __ONEGUE_SERIALIZE_H__
#define __ONEGUE_SERIALIZE_H__

#include <stdint.h>
#include "../data_structures/hash_table.h"
#include "../data_structures/ranged_binary_tree.h"
#include "../file/file.h"

#define ONEGUI_DATA_HEADER  0x44415441

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

struct ModuleExports {
    struct NamedExport* typeExports;
    int typeCount;
    struct NamedExport* valueExports;
    int valueCount;
};

void oneGuiSerializeWithState(struct SerializerState* state, struct ModuleExports* exports, struct OGFile* output);

void oneGuiSerializeExports(struct ModuleExports* exports, struct OGFile output);

#endif