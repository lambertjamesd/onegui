#include "module.h"
#include "../types/type_builder.h"

struct DataType* _gOGModuleType;

void _ogModuleInitTypes() {
    if (_gOGModuleType) {
        return;
    }

    _gOGModuleType = typeBuilderNewObject(sizeof(struct OGModule), 2);
    TYPE_BUILDER_APPEND_SUB_TYPE(_gOGModuleType, struct OGModule, name, (struct DataType*)typeBuilderGetStringType());
    TYPE_BUILDER_APPEND_SUB_TYPE(_gOGModuleType, struct OGModule, exports, typeBuilderGetPointerToUnknown());
}

struct OGModule* ogModuleNew(OString name) {
    _ogModuleInitTypes();
    struct OGModule* result = refMalloc(_gOGModuleType);
    result->name = refRetain(name);
    result->exports = hashTableNew(hashTableStringEquality, hashTableStringHash, HashTableFlagsRetainKey | HashTableFlagsRetainValue);
    return result;
}

void ogModuleAppend(struct OGModule* module, OString name, void* ref) {
    hashTableSetKV(module->exports, name, ref);
}

void ogModuleAppendExports(struct OGModule* module, struct ModuleExports* exports) {
    for (unsigned i = 0; exports->types && i < exports->types->header.count; ++i) {
        struct NamedExport* namedExport = &exports->types->data[i];
        hashTableSetKV(module->exports, namedExport->name, namedExport->exportValue);
    }

    for (unsigned i = 0; exports->values && i < exports->values->header.count; ++i) {
        struct NamedExport* namedExport = (struct NamedExport*)&exports->values->data[i];
        hashTableSetKV(module->exports, namedExport->name, namedExport->exportValue);
    }
}