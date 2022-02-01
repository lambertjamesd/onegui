#include "exports.h"
#include "../types/type_builder.h"
#include <assert.h>

struct DataType* _gSerializerStateType;
struct DataType* _gObjectExportInformationType;
struct DynamicArrayDataType* gNamedExportArrayType;

struct SerializerState* gSerializerState;

void oneGuiExportsInit() {
    _gSerializerStateType = typeBuilderNewObject(sizeof(struct SerializerState), 2);
    TYPE_BUILDER_APPEND_SUB_TYPE(_gSerializerStateType, struct SerializerState, addressToObjectInfo, typeBuilderGetPointerToUnknown());
    TYPE_BUILDER_APPEND_SUB_TYPE(_gSerializerStateType, struct SerializerState, objectNameToObject, typeBuilderGetPointerToUnknown());

    gSerializerState = refMalloc(_gSerializerStateType);
    gSerializerState->addressToObjectInfo = rangedBinaryTreeNew();
    gSerializerState->objectNameToObject = hashTableNew(hashTableBasicEquality, hashTableIntegerHash, HashTableFlagsRetainKey | HashTableFlagsRetainValue);

    _gObjectExportInformationType = typeBuilderNewObject(sizeof(struct ObjectExportInformation), 3);
    TYPE_BUILDER_APPEND_SUB_TYPE(_gObjectExportInformationType, struct ObjectExportInformation, objectRef, typeBuilderGetPointerToUnknown());
    TYPE_BUILDER_APPEND_SUB_TYPE(_gObjectExportInformationType, struct ObjectExportInformation, moduleName, (struct DataType*)typeBuilderGetStringType());
    TYPE_BUILDER_APPEND_SUB_TYPE(_gObjectExportInformationType, struct ObjectExportInformation, name, (struct DataType*)typeBuilderGetStringType());

    struct DataType* namedExportType = typeBuilderNewObject(sizeof(struct NamedExport), 2);
    TYPE_BUILDER_APPEND_SUB_TYPE(namedExportType, struct NamedExport, name, (struct DataType*)typeBuilderGetStringType());
    TYPE_BUILDER_APPEND_SUB_TYPE(namedExportType, struct NamedExport, exportValue, typeBuilderGetPointerToUnknown());

    gNamedExportArrayType = typeBuilderNewVariableArray(namedExportType);

    refRelease(namedExportType);
}

void oneGuiExportsAppend(OString moduleName, struct ModuleExports* exports) {
    for (unsigned i = 0; i < exports->typeCount; ++i) {
        struct ObjectExportInformation* exportInfo = refMalloc(_gObjectExportInformationType);
        struct NamedExport* namedExport = &exports->typeExports[i];

        exportInfo->objectRef = refRetain(namedExport->exportValue);
        exportInfo->moduleName = refRetain(moduleName);
        exportInfo->name = refRetain(namedExport->name);
        
        rangedBinaryTreeInsert(gSerializerState->addressToObjectInfo, (uint64_t)namedExport->exportValue, refSize(namedExport->exportValue), exportInfo);
        refRelease(exportInfo);
    }

    for (unsigned i = 0; i < exports->valueCount; ++i) {
        struct ObjectExportInformation* exportInfo = refMalloc(_gObjectExportInformationType);
        struct NamedExport* namedExport = (struct NamedExport*)&exports->valueExports[i];

        exportInfo->objectRef = refRetain(namedExport->exportValue);
        exportInfo->moduleName = refRetain(moduleName);
        exportInfo->name = refRetain(namedExport->name);

        rangedBinaryTreeInsert(gSerializerState->addressToObjectInfo, (uint64_t)namedExport, refSize(namedExport->exportValue), exportInfo);
        refRelease(exportInfo);
    }
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