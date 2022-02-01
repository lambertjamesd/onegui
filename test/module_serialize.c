#include "test.h"

#include <stdio.h>

#include "../src/file/file.h"
#include "../src/types/types.h"
#include "../src/types/type_builder.h"
#include "../src/ref/ref.h"
#include "../src/module/exports.h"
#include "../src/types/string.h"
#include "../src/module/serialize.h"

int ogFileNativeWrite(void* file, const char* data, int count) {
    return fwrite(data, count, 1, file);
}

int ogFileNativeRead(void* file, char* data, int count) {
    return fread(data, count, 1, file);
}

int gWenceMapping[] = {
    [SeekTypeSet] = SEEK_SET,
    [SeekTypeCurr] = SEEK_CUR,
    [SeekTypeEnd] = SEEK_END,
};

int64_t ogFileNativeSeek(void* file, int64_t offset, enum SeekType seekType) {
    fseek(file, offset, gWenceMapping[seekType]);
    return ftell(file);
}

void ogFileNativeClose(void* file) {
    fclose(file);
}

struct FileInterface gFileInterface = {
    .write = ogFileNativeWrite,
    .read = ogFileNativeRead,
    .seek = ogFileNativeSeek,
    .close = ogFileNativeClose,
};

bool ogFileNativeOpen(struct OGFile* file, const char* filename, const char* mode) {
    file->interface = &gFileInterface;
    file->file = fopen(filename, mode);

    return file->file != NULL;
}

struct TestStructure {
    OString name;
    uint32_t value;
};

void testModuleSerialize() {
    struct DataType* testStructureType = typeBuilderNewObject(sizeof(struct TestStructure), 2);
    TYPE_BUILDER_APPEND_SUB_TYPE(testStructureType, struct TestStructure, name, (struct DataType*)typeBuilderGetStringType());
    TYPE_BUILDER_APPEND_SUB_TYPE(testStructureType, struct TestStructure, value, typeBuilderGetPrimitive(DataTypeUInt32));

    struct NamedExportArray* typeExports = (struct NamedExportArray*)refMallocArray(gNamedExportArrayType, 1);
    namedExportAppend(typeExports, OSTR_NEW_FROM_CSTR("TestStructure"), testStructureType);

    struct TestStructure* test = refMalloc(testStructureType);

    struct NamedExportArray* dataExports = (struct NamedExportArray*)refMallocArray(gNamedExportArrayType, 1);
    namedExportAppend(dataExports, OSTR_NEW_FROM_CSTR("gTestStructure"), test);

    struct ModuleExports moduleExports;

    moduleExports.types = typeExports;
    moduleExports.values = dataExports;

    struct OGFile file;
    TEST_ASSERT(ogFileNativeOpen(&file, "test.ogui", "wb"));

    oneGuiSerializeExports(&moduleExports, &file);

    ogFileClose(&file);

    refRelease(typeExports);
    refRelease(dataExports);
    refRelease(testStructureType);
}