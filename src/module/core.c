#include "core.h"

#include "../ref/ref.h"
#include "../types/string.h"
#include "exports.h"
#include "../types/type_builder.h"
#include "../ref/basic_types.h"

void coreModuleInit() {
    struct NamedExportArray* typeExports = (struct NamedExportArray*)refMallocArray(gNamedExportArrayType, 24);

    namedExportAppend(typeExports, OSTR_NEW_FROM_CSTR("u8"), typeBuilderGetPrimitive(DataTypeUInt8));
    namedExportAppend(typeExports, OSTR_NEW_FROM_CSTR("i8"), typeBuilderGetPrimitive(DataTypeInt8));
    namedExportAppend(typeExports, OSTR_NEW_FROM_CSTR("u16"), typeBuilderGetPrimitive(DataTypeUInt16));
    namedExportAppend(typeExports, OSTR_NEW_FROM_CSTR("i16"), typeBuilderGetPrimitive(DataTypeInt16));
    namedExportAppend(typeExports, OSTR_NEW_FROM_CSTR("u32"), typeBuilderGetPrimitive(DataTypeUInt32));
    namedExportAppend(typeExports, OSTR_NEW_FROM_CSTR("i32"), typeBuilderGetPrimitive(DataTypeInt32));
    namedExportAppend(typeExports, OSTR_NEW_FROM_CSTR("u64"), typeBuilderGetPrimitive(DataTypeUInt64));
    namedExportAppend(typeExports, OSTR_NEW_FROM_CSTR("i64"), typeBuilderGetPrimitive(DataTypeInt64));

    namedExportAppend(typeExports, OSTR_NEW_FROM_CSTR("f32"), typeBuilderGetPrimitive(DataTypeFloat32));
    namedExportAppend(typeExports, OSTR_NEW_FROM_CSTR("f64"), typeBuilderGetPrimitive(DataTypeFloat64));

    namedExportAppend(typeExports, OSTR_NEW_FROM_CSTR("string"), (struct DataType*)typeBuilderGetStringType());

    namedExportAppend(typeExports, OSTR_NEW_FROM_CSTR("ObjectDataType"), (struct DataType*)(typeBuilderBasicTypes()->objectDataType));
    namedExportAppend(typeExports, OSTR_NEW_FROM_CSTR("ObjectSubType"), (struct DataType*)(typeBuilderBasicTypes()->objectSubType));
    namedExportAppend(typeExports, OSTR_NEW_FROM_CSTR("PrimitiveDataType"), (struct DataType*)(typeBuilderBasicTypes()->primitiveDataType));
    namedExportAppend(typeExports, OSTR_NEW_FROM_CSTR("PointerDataType"), (struct DataType*)(typeBuilderBasicTypes()->pointerDataType));
    namedExportAppend(typeExports, OSTR_NEW_FROM_CSTR("DynamicArrayDataType"), (struct DataType*)(typeBuilderBasicTypes()->dynamicArrayDataType));
    namedExportAppend(typeExports, OSTR_NEW_FROM_CSTR("FixedArrayDataType"), (struct DataType*)(typeBuilderBasicTypes()->fixedArrayDataType));

    namedExportAppend(typeExports, OSTR_NEW_FROM_CSTR("unknown"), (struct DataType*)(typeBuilderBasicTypes()->unknownType));

    struct ModuleExports moduleExports;

    moduleExports.types = typeExports;
    moduleExports.values = NULL;

    oneGuiExportsAppend(OSTR_NEW_FROM_CSTR("core"), &moduleExports);

    refRelease(typeExports);
}