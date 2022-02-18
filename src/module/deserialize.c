#include "deserialize.h"
#include "../error/error.h"
#include "../types/string.h"

struct OGDeserializeState {
    struct NamedExportArray* types;
    struct NamedExportArray* values;
};

bool  _oneGuiReadVarUint(struct OGFile* input, int nBits, uint64_t* output, int* nBytesOut) {
    uint64_t result = 0;
    uint8_t curr;

    int currBits = 0;
    int nBytes = 0;

    do {
        ogFileRead(input, &curr, 1);
        result = (result << 7) | (curr & 0x7F);
        currBits += 7;
        ++nBytes;
    } while ((result & 0x80) != 0 && currBits < nBits);

    if (currBits > nBits) {
        return false;
    }

    if (output) {
        *output = result;
    }

    if (nBytesOut) {
        *nBytesOut = nBytes;
    }

    return true;
}

bool _oneGuiSkipString(struct OGFile* input) {
    uint64_t length;

    if (!_oneGuiReadVarUint(input, 32, &length, NULL)) {
        return false;
    }

    ogFileSeek(input, length, SeekTypeCurr);

    return true;
}

OString _oneGuiReadString(struct OGFile* input) {
    uint64_t length;
    int lengthBytes;

    if (!_oneGuiReadVarUint(input, 32, &length, &lengthBytes)) {
        return NULL;
    }

    OString result = ostrNewFromLen((uint32_t)length);
    ogFileRead(input, result + lengthBytes, (uint32_t)length);
    return result;
}

bool _oneGuiDeserializeImports(struct SerializerState* state, struct OGFile* input, struct NamedExportArray* output, uint32_t importCount) {
    for (uint32_t i = 0; i < importCount; ++i) {
        OString moduleName = _oneGuiReadString(input);
        OString exportName = _oneGuiReadString(input);

        struct OGModule* module;

        if (!hashTableGetKV(state->modules, moduleName, (void**)&module)) {
            ogReportError("_oneGuiDeserializeImports: could not find module");
            return false;
        }

        void* export;

        if (!hashTableGetKV(module->exports, exportName, &export)) {
            ogReportError("_oneGuiDeserializeImports: could not find export");
            return false;
        }

        namedExportAppend(output, NULL, export);

        refRelease(moduleName);
        refRelease(exportName);
    }

    return true;
}

bool _oneGuiCreateType(struct SerializerState* state, struct OGDeserializeState* deserializeState, struct OGFile* input, uint32_t index) {
    uint16_t type;
    uint16_t flags;

    OString name = _oneGuiReadString(input);
    deserializeState->types->data[index].name = refRetain(name);
    refRelease(name);

    ogFileRead(input, &type, sizeof(uint16_t));
    ogFileRead(input, &flags, sizeof(uint16_t));

    struct DataType* ref = NULL;
    struct DataType* unknownType = typeBuilderGetUnknown();

    switch (type) {
        case DataTypeWeakPointer:
            ref = typeBuilderNewWeakPointerType(unknownType);
            break;
        case DataTypePointer:
            ref = typeBuilderNewPointerType(unknownType);
            break;
        case DataTypeFixedArray:
        {
            uint32_t elementCount;
            ogFileRead(input, &elementCount, sizeof(uint32_t));
            ref = typeBuilderNewFixedArray(unknownType, elementCount);
            break;
        }
        case DataTypeDynamicArray:
            ref = (struct DataType*)typeBuilderNewVariableArray(unknownType);
            break;
        case DataTypeObject:
        {
            uint32_t byteSize;
            uint32_t elementCount;

            ogFileRead(input, &byteSize, sizeof(uint32_t));
            ogFileRead(input, &elementCount, sizeof(uint32_t));

            ref = typeBuilderNewObject(byteSize, elementCount);
            break;
        }
        default:
            return false;
    }

    ref->flags |= flags;
    deserializeState->types->data[index].exportValue = ref;

    return true;
}

bool _oneGuiReadTypeRef(struct OGDeserializeState* deserializeState, struct OGFile* input, struct DataType** output) {
    uint32_t index;
    ogFileRead(input, &index, sizeof(uint32_t));

    if (index == 0) {
        *output = NULL;
        return true;
    }

    --index;

    if (index >= deserializeState->types->header.count) {
        return false;
    }

    *output = deserializeState->types->data[index].exportValue;

    return true;
}

bool _oneGuiFillType(struct SerializerState* state, struct OGDeserializeState* deserializeState, struct OGFile* input, uint32_t index) {
    _oneGuiSkipString(input);

    ogFileSeek(input, sizeof(uint16_t) + sizeof(uint16_t), SeekTypeCurr);

    struct DataType* ref = deserializeState->types->data[index].exportValue;

    switch (ref->type) {
        case DataTypeWeakPointer:
        case DataTypePointer:
        {
            struct PointerDataType* pointerType = (struct PointerDataType*)ref;
            struct DataType* subType;
            if (!_oneGuiReadTypeRef(deserializeState, input, &subType)) {
                return false;
            }

            refRelease(pointerType->subType);
            pointerType->subType = refRetain(subType);
            break;
        }
        case DataTypeFixedArray:
        {
            struct FixedArrayDataType* arrayType = (struct FixedArrayDataType*)ref;
            // skip element count
            ogFileSeek(input, sizeof(uint32_t), SeekTypeCurr);
            struct DataType* subType;
            if (!_oneGuiReadTypeRef(deserializeState, input, &subType)) {
                return false;
            }
            refRelease(arrayType->subType);
            arrayType->subType = refRetain(subType);
            break;
        }
        case DataTypeDynamicArray:
        {
            struct DynamicArrayDataType* arrayType = (struct DynamicArrayDataType*)ref;
            struct DataType* subType;
            if (!_oneGuiReadTypeRef(deserializeState, input, &subType)) {
                return false;
            }

            refRelease(arrayType->subType);
            arrayType->subType = refRetain(subType);
            break;
        }
        case DataTypeObject:
        {
            // skip byteSize and elementCount
            ogFileSeek(input, sizeof(uint32_t) + sizeof(uint32_t), SeekTypeCurr);
            struct ObjectDataType* objectType = (struct ObjectDataType*)ref;

            for (uint32_t i = 0; i < objectType->objectSubTypes->header.capacity; ++i) {
                OString name = _oneGuiReadString(input);
                struct DataType* subType;
                if (!_oneGuiReadTypeRef(deserializeState, input, &subType)) {
                    return false;
                }
                uint32_t offset;
                ogFileRead(input, &offset, sizeof(uint32_t));
                
                struct ObjectSubType* subTypePtr = &objectType->objectSubTypes->elements[i];

                subTypePtr->name = name;
                subTypePtr->offset -= offset;
                subTypePtr->type = refRetain(subType);
            }

            objectType->objectSubTypes->header.count = objectType->objectSubTypes->header.capacity;

            break;
        }
        default:
            return false;
    }

    return true;
}

typedef bool (*EntryIterator)(struct SerializerState* state, struct OGDeserializeState* deserializeState, struct OGFile* input, uint32_t index);

bool _oneGuiIterateEntries(struct SerializerState* state, struct OGDeserializeState* deserializeState, struct OGFile* input, uint32_t dataCount, uint64_t fileStart, EntryIterator iterator, uint32_t indexOffset) {
    uint64_t curr = fileStart;
    uint32_t index = 0;

    while (index < dataCount) {
        ogFileSeek(input, curr, SeekTypeSet);
        uint32_t chunkSize;
        ogFileRead(input, &chunkSize, sizeof(uint32_t));

        if (!iterator(state, deserializeState, input, index + indexOffset)) {
            return false;
        }
        
        curr += chunkSize + sizeof(uint32_t);
        ++index;
    }

    return true;
}

bool _oneGuiReadTypes(struct SerializerState* state, struct OGDeserializeState* deserializeState, struct OGFile* input) {
    uint32_t importCount;
    uint32_t dataCount;

    ogFileRead(input, &importCount, sizeof(uint32_t));
    ogFileRead(input, &dataCount, sizeof(uint32_t));

    deserializeState->types = namedExportArrayNew(importCount + dataCount);
    if (!_oneGuiDeserializeImports(state, input, deserializeState->types, importCount)) {
        refRelease(deserializeState->types);
        return false;
    }
    deserializeState->types->header.count = deserializeState->types->header.capacity;

    uint64_t dataListStart = ogFileSeek(input, 0, SeekTypeCurr);

    _oneGuiIterateEntries(state, deserializeState, input, dataCount, dataListStart, _oneGuiCreateType, importCount);
    _oneGuiIterateEntries(state, deserializeState, input, dataCount, dataListStart, _oneGuiFillType, importCount);

    return true;
}

bool _oneGuiCreateData(struct SerializerState* state, struct OGDeserializeState* deserializeState, struct OGFile* input, uint32_t index) {
    OString name = _oneGuiReadString(input);
    deserializeState->values->data[index].name = name;
    refRelease(name);

    struct DataType* type;
    if (!_oneGuiReadTypeRef(deserializeState, input, &type)) {
        return false;
    }

    void* ref;

    if (type->type == DataTypeDynamicArray) {
        struct DynamicArrayHeader arrayHeader;
        ogFileRead(input, &arrayHeader, sizeof(struct DynamicArrayHeader));
        struct DynamicArray* asArray = refMallocArray((struct DynamicArrayDataType*)type, arrayHeader.capacity);
        asArray->header.count = arrayHeader.count;
        ref = asArray;
    } else {
        ref = refMalloc(type);
    }

    deserializeState->values->data[index].exportValue = ref;

    return true;
}

bool _oneGuiReadStrongRef(struct OGDeserializeState* deserializeState, struct OGFile* input, void** output) {
    uint32_t index;
    ogFileRead(input, &index, sizeof(uint32_t));

    if (input == 0) {
        *output = NULL;
        return true;
    }

    --index;

    if (index >= deserializeState->values->header.count) {
        return false;
    }

    *output = &deserializeState->values->data[index];
    return true;
}

bool _oneGuiFillDataWithType(struct SerializerState* state, struct OGDeserializeState* deserializeState, struct OGFile* input, void* ref, struct DataType* type);

bool _oneGuiFillObjectWithType(struct SerializerState* state, struct OGDeserializeState* deserializeState, struct OGFile* input, void* ref, struct ObjectDataType* type) {
    char* start = ref;
    unsigned lastRead = 0;

    for (uint32_t i = 0; i < type->objectSubTypes->header.count; ++i) {
        struct ObjectSubType* subType = &type->objectSubTypes->elements[i];
        if (subType->type->flags & (DataTypeFlagsHasStrongRef | DataTypeFlagsHasWeakRef)) {
            unsigned toRead = subType->offset - lastRead;

            if (toRead) {
                ogFileRead(input, start + lastRead, subType->offset - lastRead);
                lastRead = subType->offset;
            }

            if (!_oneGuiFillDataWithType(state, deserializeState, input, start + subType->offset, subType->type)) {
                return false;
            }
            
            lastRead += dataTypeSize(subType->type);
        }
    }

    unsigned objectSize = dataTypeSize((struct DataType*)type);

    if (lastRead < objectSize) {
        ogFileRead(input, start + lastRead, objectSize - lastRead);
    }

    return true;
}

bool _oneGuiFillDataWithType(struct SerializerState* state, struct OGDeserializeState* deserializeState, struct OGFile* input, void* ref, struct DataType* type) {
    switch (type->type) {
        case DataTypeString:
            *((OString*)ref) = _oneGuiReadString(input);
            break;
        case DataTypePointer:
        {
            void* pointTo;
            if (!_oneGuiReadStrongRef(deserializeState, input, &pointTo)) {
                return false;
            }

            *((void**)ref) = refRetain(pointTo);
            break;
        }
        case DataTypeWeakPointer:
        {
            void* pointTo;
            if (!_oneGuiReadStrongRef(deserializeState, input, &pointTo)) {
                return false;
            }
            uint32_t offset;
            ogFileRead(input, &offset, sizeof(uint32_t));

            *((void**)ref) = (char*)pointTo + offset;
            break;
        }
        case DataTypeFixedArray:
        {
            struct FixedArrayDataType* typeAsArray = (struct FixedArrayDataType*)type;

            // skip header
            ogFileSeek(input, sizeof(struct DynamicArrayHeader), SeekTypeCurr);

            char* curr = ref;
            uint32_t elementSize = dataTypeSize(typeAsArray->subType);

            for (uint32_t i = 0; i < typeAsArray->elementCount; ++i) {
                if (!_oneGuiFillDataWithType(state, deserializeState, input, curr, typeAsArray->subType)) {
                    return false;
                }
                curr += elementSize;
            }
            break;
        }
        case DataTypeDynamicArray:
        {
            struct DynamicArrayDataType* typeAsArray = (struct DynamicArrayDataType*)type;
            struct DynamicArray* dataAsArray = (struct DynamicArray*)ref;

            // skip header
            ogFileSeek(input, sizeof(struct DynamicArrayHeader), SeekTypeCurr);

            char* curr = &dataAsArray->data[0];
            uint32_t elementSize = dataTypeSize(typeAsArray->subType);

            for (uint32_t i = 0; i < dataAsArray->header.count; ++i) {
                if (!_oneGuiFillDataWithType(state, deserializeState, input, curr, typeAsArray->subType)) {
                    return false;
                }
                curr += elementSize;
            }
            break;
        }
        case DataTypeObject:
            return _oneGuiFillObjectWithType(state, deserializeState, input, ref, (struct ObjectDataType*)type);
        default: 
            return false;
    }
    
    return true;
}

bool _oneGuiFillData(struct SerializerState* state, struct OGDeserializeState* deserializeState, struct OGFile* input, uint32_t index) {
    _oneGuiSkipString(input);

    // skip over the type reference
    ogFileSeek(input, sizeof(uint32_t), SeekTypeCurr);

    void* ref = &deserializeState->values->data[index];
    struct DataType* type = refGetDataType(ref);
    
    if (!(type->flags & (DataTypeFlagsHasStrongRef | DataTypeFlagsHasWeakRef))) {
        if (type->type == DataTypeDynamicArray) {
            struct DynamicArrayDataType* typeAsArray = (struct DynamicArrayDataType*)type;
            struct DynamicArray* dataAsArray = (struct DynamicArray*)ref;
            ogFileRead(input, ref, sizeof(struct DynamicArrayHeader) + dataTypeSize(typeAsArray->subType) * dataAsArray->header.count);
        } else {
            ogFileRead(input, ref, dataTypeSize(type));
        }

        return true;
    }

    return _oneGuiFillDataWithType(state, deserializeState, input, ref, type);
}

bool _oneGuiReadData(struct SerializerState* state, struct OGDeserializeState* deserializeState, struct OGFile* input) {
    uint32_t importCount;
    uint32_t dataCount;

    ogFileRead(input, &importCount, sizeof(uint32_t));
    ogFileRead(input, &dataCount, sizeof(uint32_t));

    deserializeState->values = namedExportArrayNew(importCount + dataCount);
    if (!_oneGuiDeserializeImports(state, input, deserializeState->values, importCount)) {
        refRelease(deserializeState->values);
        return false;
    }
    deserializeState->values->header.count = deserializeState->values->header.capacity;

    uint64_t dataListStart = ogFileSeek(input, 0, SeekTypeCurr);

    _oneGuiIterateEntries(state, deserializeState, input, dataCount, dataListStart, _oneGuiCreateData, importCount);
    _oneGuiIterateEntries(state, deserializeState, input, dataCount, dataListStart, _oneGuiFillData, importCount);

    return true;
}

bool oneGuiDeserializeWithState(struct SerializerState* state, struct OGFile* input, struct ModuleExports** output) {
    uint16_t version;
    ogFileRead(input, &version, sizeof(uint16_t));

    if (version != 0) {
        ogReportError("oneGuiDeserializeWithState: Invalid file version");
        return false;
    }

    struct OGDeserializeState deserializeState;

    if (!_oneGuiReadTypes(state, &deserializeState, input)) {
        return false;
    }

    if (!_oneGuiReadData(state, &deserializeState, input)) {
        refRelease(deserializeState.types);
        return false;
    }

    struct NamedExportArray* types = namedExportsFilterUnamed(deserializeState.types);
    struct NamedExportArray* values = namedExportsFilterUnamed(deserializeState.values);

    refRelease(deserializeState.types);
    refRelease(deserializeState.values);

    *output = moduleExportsNew(types, values);

    refRelease(types);
    refRelease(values);

    return true;
}

bool oneGuiDeserialize(struct SerializerState* state, struct OGFile* input, struct ModuleExports** output) {
    return oneGuiDeserializeWithState(gSerializerState, input, output);
}