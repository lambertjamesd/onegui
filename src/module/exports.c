#include "exports.h"
#include "../types/type_builder.h"
#include "../types/string.h"
#include <assert.h>

struct DataType* _gSerializerStateType;
struct DynamicArrayDataType* gNamedExportArrayType;
struct DataType* gModuleExportsType;
struct DataType* _gOGExportInfoType;

struct SerializerState* gSerializerState;

void oneGuiExportsInit() {
    _gSerializerStateType = typeBuilderNewObject(sizeof(struct SerializerState), 2);
    TYPE_BUILDER_APPEND_SUB_TYPE(_gSerializerStateType, struct SerializerState, addressToObjectInfo, typeBuilderGetPointerToUnknown());
    TYPE_BUILDER_APPEND_SUB_TYPE(_gSerializerStateType, struct SerializerState, modules, typeBuilderGetPointerToUnknown());

    gSerializerState = refMalloc(_gSerializerStateType);
    gSerializerState->addressToObjectInfo = rangedBinaryTreeNew();
    gSerializerState->modules = hashTableNew(hashTableBasicEquality, hashTableIntegerHash, HashTableFlagsRetainKey | HashTableFlagsRetainValue);

    struct DataType* namedExportType = typeBuilderNewObject(sizeof(struct NamedExport), 2);
    TYPE_BUILDER_APPEND_SUB_TYPE(namedExportType, struct NamedExport, name, (struct DataType*)typeBuilderGetStringType());
    TYPE_BUILDER_APPEND_SUB_TYPE(namedExportType, struct NamedExport, exportValue, typeBuilderGetPointerToUnknown());

    gNamedExportArrayType = typeBuilderNewVariableArray(namedExportType);

    refRelease(namedExportType);

    struct DataType* pointerToArray = typeBuilderNewPointerType((struct DataType*)gNamedExportArrayType);

    gModuleExportsType = typeBuilderNewObject(sizeof(struct ModuleExports), 2);
    TYPE_BUILDER_APPEND_SUB_TYPE(gModuleExportsType, struct ModuleExports, types, pointerToArray);
    TYPE_BUILDER_APPEND_SUB_TYPE(gModuleExportsType, struct ModuleExports, values, pointerToArray);

    refRelease(pointerToArray);

    _gOGExportInfoType = typeBuilderNewObject(sizeof(struct OGExportInfo), 3);
    TYPE_BUILDER_APPEND_SUB_TYPE(_gOGExportInfoType, struct OGExportInfo, objectRef, typeBuilderGetPointerToUnknown());
    TYPE_BUILDER_APPEND_SUB_TYPE(_gOGExportInfoType, struct OGExportInfo, moduleName, (struct DataType*)typeBuilderGetStringType());
    TYPE_BUILDER_APPEND_SUB_TYPE(_gOGExportInfoType, struct OGExportInfo, name, (struct DataType*)typeBuilderGetStringType());
}

void ogExportsAddModule(struct OGModule* module) {
    struct HashTableIterator it;
    hashTableIteratorInit(&it, module->exports);

    for (struct HashEntry* entry; (entry = hashTableIteratorCurrent(&it)); hashTableIteratorNext(&it)) {
        struct OGExportInfo* exportInfo = refMalloc(_gOGExportInfoType);

        exportInfo->objectRef = refRetain((void*)entry->value);
        exportInfo->moduleName = refRetain(module->name);
        exportInfo->name = refRetain((OString)entry->key);

        rangedBinaryTreeInsert(gSerializerState->addressToObjectInfo, entry->value, refSize((void*)entry->value), exportInfo);
        refRelease(exportInfo);
    }

    hashTableSetKV(gSerializerState->modules, module->name, module);
}

struct ModuleExports* moduleExportsNew(struct NamedExportArray* types, struct NamedExportArray* values) {
    struct ModuleExports* result = refMalloc(gModuleExportsType);
    result->types = refRetain(types);
    result->values = refRetain(values);
    return result;
}

struct NamedExportArray* namedExportArrayNew(int capacity) {
    return (struct NamedExportArray*)refMallocArray(gNamedExportArrayType, capacity);
}

void namedExportAppend(struct NamedExportArray* array, OString name, void* exportValue) {
    if (array->header.count >= array->header.capacity) {
        return;
    }

    struct NamedExport* namedExport = &array->data[array->header.count];

    namedExport->name = refRetain(name);
    namedExport->exportValue = refRetain(exportValue);

    array->header.count++;
}

struct NamedExportArray* namedExportsFilterUnamed(struct NamedExportArray* array) {
    uint32_t resultCount = 0;

    for (uint32_t i = 0; i < array->header.count; ++i) {
        if (ostrLen(array->data[i].name)) {
            ++resultCount;
        }
    }

    struct NamedExportArray* result = namedExportArrayNew(resultCount);

    for (uint32_t i = 0; i < array->header.count; ++i) {
        if (ostrLen(array->data[i].name)) {
            namedExportAppend(result, array->data[i].name, array->data[i].exportValue);
        }
    }

    return result;
}