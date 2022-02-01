#include "serialize.h"

#include <stdint.h>
#include <malloc.h>
#include "../types/type_builder.h"
#include "../types/string.h"


struct DataType* _gSerializedObjectInfoType;

enum SerializedObjectInfoFlags {
    SerializedObjectInfoFlagsIsSerialized = (1 << 0),
};

struct SerializedObjectInfo {
    void* objectRef;
    OString moduleName;
    OString name;
    uint32_t serializedIndex;
    uint32_t flags;
};

struct DeferredWeakLink {
    void* weakPointerTo;
    uint64_t fileOffset;
};

struct DeferredWeakLinkArray {
    struct DynamicArrayHeader header;
    struct DeferredWeakLink links[];
};

struct SerializedChunkState {
    struct HashTable* typeToObjectInfo;
    struct RangedBinaryTree* objectToObjectInfo;
};

void oneGuiSerializeInit() {
    _gSerializedObjectInfoType = typeBuilderNewObject(sizeof(struct SerializedObjectInfo), 5);
    TYPE_BUILDER_APPEND_SUB_TYPE(_gSerializedObjectInfoType, struct SerializedObjectInfo, objectRef, typeBuilderGetPointerToUnknown());
    TYPE_BUILDER_APPEND_SUB_TYPE(_gSerializedObjectInfoType, struct SerializedObjectInfo, moduleName, (struct DataType*)typeBuilderGetStringType());
    TYPE_BUILDER_APPEND_SUB_TYPE(_gSerializedObjectInfoType, struct SerializedObjectInfo, name, (struct DataType*)typeBuilderGetStringType());
    TYPE_BUILDER_APPEND_SUB_TYPE(_gSerializedObjectInfoType, struct SerializedObjectInfo, serializedIndex, typeBuilderGetPrimitive(DataTypeUInt32));
    TYPE_BUILDER_APPEND_SUB_TYPE(_gSerializedObjectInfoType, struct SerializedObjectInfo, flags, typeBuilderGetPrimitive(DataTypeUInt32));
}

void _oneGuiWriteString(OString string, struct OGFile* output) {
    int len;
    const char* strStart = ostrToCStr(string, &len);
    
    if (!len) {
        ogFileWrite(output, &len, 1);
        return;
    }

    ogFileWrite(output, string, (strStart - string) + len);
}

struct SerializedObjectInfo* _oneGuiInsertType(struct SerializerState* state, struct SerializedChunkState* chunkState, OString optName, struct DataType* type) {
    struct SerializedObjectInfo* typeInfo;

    if (hashTableGetKV(chunkState->typeToObjectInfo, type, (void**)&typeInfo)) {
        return typeInfo;
    }

    typeInfo = refMalloc(_gSerializedObjectInfoType);
    hashTableSetKV(chunkState->typeToObjectInfo, type, typeInfo);
    refRelease(typeInfo);

    // this will be assigned the correct value later
    typeInfo->serializedIndex = ~0;

    struct RangedBinaryTreeNode* namedType;
    // check to see if the object type is exported from another module
    if (rangedBinaryTreeGet(state->addressToObjectInfo, (uint64_t)type, &namedType)) {
        struct ObjectExportInformation* exportInfo = (struct ObjectExportInformation*)namedType->value;
        typeInfo->objectRef = refRetain(exportInfo->objectRef);
        typeInfo->moduleName = refRetain(exportInfo->moduleName);
        typeInfo->name = refRetain(exportInfo->name);
        return typeInfo;
    }

    typeInfo->objectRef = refRetain(type);
    typeInfo->name = refRetain(optName);

    switch (type->type) {
        case DataTypePointer:
            _oneGuiInsertType(state, chunkState, NULL, ((struct PointerDataType*)type)->subType);
            break;
        case DataTypeWeakPointer:
            _oneGuiInsertType(state, chunkState, NULL, ((struct PointerDataType*)type)->subType);
            break;
        case DataTypeFixedArray:
            _oneGuiInsertType(state, chunkState, NULL, ((struct FixedArrayDataType*)type)->subType);
            break;
        case DataTypeDynamicArray:
            _oneGuiInsertType(state, chunkState, NULL, ((struct DynamicArrayDataType*)type)->subType);
            break;
        case DataTypeObject:
        {
            struct ObjectDataType* objectType = (struct ObjectDataType*)type;

            for (unsigned i = 0; i < objectType->objectSubTypes->header.count; ++i) {
                _oneGuiInsertType(state, chunkState, NULL, objectType->objectSubTypes->elements[i].type);
            }
            break;
        }
    }

    return typeInfo;
}

struct SerializedObjectInfo* _oneGuiAddReference(struct SerializerState* state, struct SerializedChunkState* chunkState, OString optName, void* ref, struct DataType* type);

void _oneGuiTraverseType(struct SerializerState* state, struct SerializedChunkState* chunkState, void* ref, struct DataType* type) {
    switch (type->type) {
        case DataTypeString:
        case DataTypePointer:
            _oneGuiAddReference(state, chunkState, NULL, *((void**)ref), ((struct PointerDataType*)type)->subType);
            break;
        case DataTypeFixedArray:
        {
            struct FixedArrayDataType* arrayType = (struct FixedArrayDataType*)type;
            unsigned elementSize = dataTypeSize(arrayType->subType);

            char* curr = ref;

            for (unsigned i = 0; i < arrayType->elementCount; ++i) {
                _oneGuiTraverseType(state, chunkState, curr, arrayType->subType);
                curr += elementSize;
            }

            break;
        }
        case DataTypeDynamicArray:
        {
            struct DynamicArrayDataType* arrayType = (struct DynamicArrayDataType*)type;
            unsigned elementSize = dataTypeSize(arrayType->subType);

            struct DynamicArray* dynamicArray = (struct DynamicArray*)ref; 

            char* curr = dynamicArray->data;

            for (unsigned i = 0; i < dynamicArray->header.count; ++i) {
                _oneGuiTraverseType(state, chunkState, curr, arrayType->subType);
                curr += elementSize;
            }

            break;
        }
        case DataTypeObject:
        {
            struct ObjectDataType* objectDataType = (struct ObjectDataType*)type;
            
            for (unsigned i = 0; i < objectDataType->objectSubTypes->header.count; ++i) {
                struct ObjectSubType* subType = &objectDataType->objectSubTypes->elements[i];

                if (subType->type->flags & DataTypeFlagsHasStrongRef) {
                    _oneGuiTraverseType(state, chunkState, (char*)ref + subType->offset, subType->type);
                }
            }

            break;
        }
    }
}

struct SerializedObjectInfo* _oneGuiAddReference(struct SerializerState* state, struct SerializedChunkState* chunkState, OString optName, void* ref, struct DataType* type) {
    if (!ref) {
        return NULL;
    }

    struct SerializedObjectInfo* result;
    struct RangedBinaryTreeNode* treeNode;

    if (rangedBinaryTreeGet(chunkState->objectToObjectInfo, (uint64_t)ref, &treeNode)) {
        return treeNode->value;
    }

    result = refMalloc(_gSerializedObjectInfoType);
    result->objectRef = refRetain(ref);
    result->moduleName = NULL;
    result->name = NULL;
    result->serializedIndex = ~0;
    result->flags = 0;
    rangedBinaryTreeInsert(chunkState->objectToObjectInfo, (uint64_t)ref, (uint32_t)refSize(ref), result);
    refRelease(result);

    _oneGuiInsertType(state, chunkState, NULL, type);

    if (rangedBinaryTreeGet(state->addressToObjectInfo, (uint64_t)ref, &treeNode)) {
        struct ObjectExportInformation* exportInfo = treeNode->value;
        result->moduleName = refRetain(exportInfo->moduleName);
        result->name = refRetain(exportInfo->name);
        return result;
    }

    result->name = refRetain(optName);

    if (!(type->flags & (DataTypeFlagsHasStrongRef))) {
        return result;
    }

    _oneGuiTraverseType(state, chunkState, ref, type);

    return result;
}

int serializedInfoCompare(struct SerializedObjectInfo* a, struct SerializedObjectInfo* b) {
    if (a->moduleName && b->moduleName) {
        int moduleCompare = ostrCompare(a->moduleName, b->moduleName);

        if (moduleCompare) {
            return moduleCompare;
        }

        return ostrCompare(a->name, b->name);
    } else if (a->moduleName) {
        return -1;
    } else if (b->moduleName) {
        return 1;
    }

    return a->serializedIndex - b->serializedIndex;
}

void _sortObjectInfo(struct SerializedObjectInfo** array, struct SerializedObjectInfo** workingMemeory, int minIndex, int maxIndex) {
    int count = maxIndex - minIndex;

    if (count <= 1) {
        return;
    }

    int mid = (maxIndex + minIndex) / 2;

    _sortObjectInfo(array, workingMemeory, minIndex, mid);
    _sortObjectInfo(array, workingMemeory, mid, maxIndex);

    int aHead = minIndex;
    int bHead = mid;

    int writeHead = minIndex;

    while (aHead < mid || bHead < maxIndex) {
        if (bHead >= maxIndex || (aHead < mid && serializedInfoCompare(array[aHead], array[bHead]) < 0)) {
            workingMemeory[writeHead++] = array[aHead++];
        } else {
            workingMemeory[writeHead++] = array[bHead++];
        }
    }

    for (writeHead = minIndex; writeHead < maxIndex; ++writeHead) {
        array[writeHead] = workingMemeory[writeHead];
    }
}

void _oneGuiSerializeWriteTypeRef(struct SerializedChunkState* chunkState, struct DataType* type, struct OGFile* output) {
    uint32_t refIndex = 0;

    struct SerializedObjectInfo* objectInfo;
    if (type && hashTableGetKV(chunkState->typeToObjectInfo, type, (void**)&objectInfo)) {
        refIndex = objectInfo->serializedIndex;
    }

    ogFileWrite(output, &refIndex, sizeof(uint32_t));
}

void _oneGuiSerializeType(struct SerializedChunkState* chunkState, struct SerializedObjectInfo* info, struct OGFile* output) {
    _oneGuiWriteString(info->name, output);
    struct DataType* type = (struct DataType*)info->objectRef;

    ogFileWrite(output, &type->type, sizeof(uint16_t));
    ogFileWrite(output, &type->flags, sizeof(uint16_t));

    switch (type->type) {
        case DataTypeWeakPointer:
        case DataTypePointer:
            _oneGuiSerializeWriteTypeRef(chunkState, ((struct PointerDataType*)type)->subType, output);
            break;
        case DataTypeFixedArray:
        {
            struct FixedArrayDataType* fixedType = (struct FixedArrayDataType*)type;
            ogFileWrite(output, &fixedType->elementCount, sizeof(uint32_t));
            _oneGuiSerializeWriteTypeRef(chunkState, fixedType->subType, output);
            break;
        }
        case DataTypeDynamicArray:
        {
            struct DynamicArrayDataType* dynamicType = (struct DynamicArrayDataType*)type;
            _oneGuiSerializeWriteTypeRef(chunkState, dynamicType->subType, output);
            break;
        }
        case DataTypeObject:
        {
            struct ObjectDataType* objectType = (struct ObjectDataType*)type;
            ogFileWrite(output, &objectType->byteSize, sizeof(uint32_t));
            ogFileWrite(output, &objectType->objectSubTypes->header.count, sizeof(uint32_t));

            for (uint32_t i = 0; i < objectType->objectSubTypes->header.count; ++i) {
                struct ObjectSubType* element = &objectType->objectSubTypes->elements[i];
                _oneGuiWriteString(element->name, output);
                _oneGuiSerializeWriteTypeRef(chunkState, element->type, output);
                ogFileWrite(output, &element->offset, sizeof(uint32_t));
            }

            break;
        }
    }
}

typedef void (*EntryWriter)(struct SerializedChunkState* chunkState, struct SerializedObjectInfo* info, struct OGFile* output);

void _oneGuiGenerateEntries(struct SerializedObjectInfo** entries, uint32_t count, struct SerializedChunkState* chunkState, EntryWriter entryWriter, struct OGFile* output) {
    for (uint32_t i = 0; i < count; ++i) {
        entries[i]->serializedIndex = i + 1;
    }

    uint32_t importCount = 0;

    while (importCount < count && entries[importCount]->moduleName) {
        ++importCount;
    }

    uint32_t dataCount = count - importCount;
    ogFileWrite(output, &importCount, sizeof(uint32_t));
    ogFileWrite(output, &dataCount, sizeof(uint32_t));

    for (uint32_t i = 0; i < importCount; ++i) {
        _oneGuiWriteString(entries[i]->moduleName, output);
        _oneGuiWriteString(entries[i]->name, output);
    }

    for (uint32_t i = importCount; i < count; ++i) {
        entryWriter(chunkState, entries[i], output);
    }
}

void _oneGuiGenerateTypes(struct SerializedChunkState* chunkState, struct OGFile* output) {
    uint32_t count = hashTableSize(chunkState->typeToObjectInfo);
    struct SerializedObjectInfo** entries = malloc(sizeof(struct SerializedObjectInfo*) * count);

    struct HashTableIterator iterator;
    hashTableIteratorInit(&iterator, chunkState->typeToObjectInfo);

    uint32_t index = 0;
    for (struct HashEntry* entry;(entry = hashTableIteratorCurrent(&iterator)); hashTableIteratorNext(&iterator)) {
        entries[index] = (struct SerializedObjectInfo*)entry->value;
        ++index;
    }

    struct SerializedObjectInfo** workingMemory = malloc(sizeof(struct SerializedObjectInfo*) * count);
    _sortObjectInfo(entries, workingMemory, 0, count);
    free(workingMemory);

    _oneGuiGenerateEntries(entries, count, chunkState, _oneGuiSerializeType, output);

    free(entries);
}

void _oneGuiWriteDataWithType(struct SerializedChunkState* chunkState, void* ref, struct DataType* dataType, struct OGFile* output);

void _oneGuiWriteObject(struct SerializedChunkState* chunkState, void* ref, struct ObjectDataType* type, struct OGFile* output) {
    char* start = ref;
    unsigned lastWritten = 0;

    for (unsigned i = 0; i < type->objectSubTypes->header.count; ++i) {
        struct ObjectSubType* subType = &type->objectSubTypes->elements[i];

        if (subType->type->flags & (DataTypeFlagsHasStrongRef | DataTypeFlagsHasWeakRef)) {
            unsigned toWrite = subType->offset - lastWritten;
            if (toWrite) {
                ogFileWrite(output, start + lastWritten, subType->offset - lastWritten);
                lastWritten = subType->offset;
            }

            _oneGuiWriteDataWithType(chunkState, start + subType->offset, subType->type, output);

            lastWritten += dataTypeSize(subType->type);
        }
    }

    unsigned objectSize = dataTypeSize((struct DataType*)type);

    if (lastWritten < objectSize) {
        ogFileWrite(output, start + lastWritten, objectSize - lastWritten);
    }
}

void _oneGuiWriteDataWithType(struct SerializedChunkState* chunkState, void* ref, struct DataType* dataType, struct OGFile* output) {
    switch (dataType->type) {
        case DataTypeString:
            _oneGuiWriteString(ref, output);
            break;
        case DataTypePointer:
        {
            struct RangedBinaryTreeNode* treeNode;
            uint32_t strongRefIndex = 0;
            void* pointerValue = *((void**)ref);
            if (pointerValue && rangedBinaryTreeGet(chunkState->objectToObjectInfo, (uint64_t)pointerValue, &treeNode)) {
                strongRefIndex = ((struct SerializedObjectInfo*)treeNode->value)->serializedIndex;
            }
            ogFileWrite(output, &strongRefIndex, sizeof(uint32_t));
            break;
        }
        case DataTypeWeakPointer:
        {
            struct RangedBinaryTreeNode* treeNode;
            uint32_t strongRefIndex = 0;
            uint32_t weakRefOffset = 0;
            if (rangedBinaryTreeGet(chunkState->objectToObjectInfo, (uint64_t)(*((void**)ref)), &treeNode)) {
                strongRefIndex = ((struct SerializedObjectInfo*)treeNode->value)->serializedIndex;
                weakRefOffset = (uint32_t)((uint64_t)ref - treeNode->min);
            }
            ogFileWrite(output, &strongRefIndex, sizeof(uint32_t));
            ogFileWrite(output, &weakRefOffset, sizeof(uint32_t));
            break;
        }
        case DataTypeFixedArray:
        {
            struct FixedArrayDataType* fixedArray = (struct FixedArrayDataType*)dataType;

            unsigned size = dataTypeSize(fixedArray->subType);
            char* curr = ref;

            for (unsigned i = 0; i < fixedArray->elementCount; ++i) {
                _oneGuiWriteDataWithType(chunkState, curr, fixedArray->subType, output);
                curr += size;
            }
            
            break;
        }
        case DataTypeDynamicArray:
        {
            struct DynamicArrayDataType* typeAsArray = (struct DynamicArrayDataType*)dataType;
            struct DynamicArray* dataAsArray = (struct DynamicArray*)ref;

            ogFileWrite(output, ref, sizeof(struct DynamicArrayHeader));

            unsigned size = dataTypeSize(typeAsArray->subType);
            char* curr = &dataAsArray->data[0];

            for (unsigned i = 0; i < dataAsArray->header.count; ++i) {
                _oneGuiWriteDataWithType(chunkState, curr, typeAsArray->subType, output);
                curr += size;
            }
            break;
        }
        case DataTypeObject:
        {
            _oneGuiWriteObject(chunkState, ref, (struct ObjectDataType*)dataType, output);
            break;
        }
    }
}

void _oneGuiSerializeData(struct SerializedChunkState* chunkState, struct SerializedObjectInfo* info, struct OGFile* output) {
    struct DataType* dataType = refGetDataType(info->objectRef);

    _oneGuiSerializeWriteTypeRef(chunkState, refGetDataType(info->objectRef), output);

    if (!(dataType->flags & (DataTypeFlagsHasStrongRef | DataTypeFlagsHasWeakRef))) {
        if (dataType->type == DataTypeDynamicArray) {
            struct DynamicArrayDataType* typeAsArray = (struct DynamicArrayDataType*)dataType;
            struct DynamicArrayHeader* dataAsArray = (struct DynamicArrayHeader*)info->objectRef;
            ogFileWrite(output, info->objectRef, sizeof(struct DynamicArrayHeader) + dataAsArray->count * dataTypeSize(typeAsArray->subType));
        } else {
            ogFileWrite(output, info->objectRef, dataTypeSize(dataType));
        }
        return;
    }

    _oneGuiWriteDataWithType(chunkState, info->objectRef, dataType, output);
}

void _oneGuiGenerateData(struct SerializedChunkState* chunkState, struct OGFile* output) {
    uint32_t count = rangedBinaryTreeSize(chunkState->objectToObjectInfo);
    struct SerializedObjectInfo** entries = malloc(sizeof(struct SerializedObjectInfo*) * count);

    struct RangedBinaryTreeIterator iterator;
    rangedBinaryTreeIteratorInit(&iterator, chunkState->objectToObjectInfo);

    uint32_t index = 0;
    for (struct RangedBinaryTreeNode* entry;(entry = rangedBinaryTreeIteratorCurrent(&iterator)); rangedBinaryTreeIteratorNext(&iterator)) {
        entries[index] = (struct SerializedObjectInfo*)entry->value;
        ++index;
    }

    struct SerializedObjectInfo** workingMemory = malloc(sizeof(struct SerializedObjectInfo*) * count);
    _sortObjectInfo(entries, workingMemory, 0, count);
    free(workingMemory);

    _oneGuiGenerateEntries(entries, count, chunkState, _oneGuiSerializeData, output);

    free(entries);
}

void _oneGuiSerializeWithState(struct SerializerState* state, struct SerializedChunkState* chunkState, struct ModuleExports* exports, struct OGFile* output) {
    // TODO move header information to the module writer
    uint32_t header = ONEGUI_DATA_HEADER;
    ogFileWrite(output, &header, sizeof(uint32_t));
    uint64_t len = 0;
    // come back and update this later
    ogFileWrite(output, &len, sizeof(uint64_t));
    uint64_t dataStart = ogFileSeek(output, 0, SeekTypeCurr);

    uint16_t version = 0;
    ogFileWrite(output, &version, sizeof(uint16_t));

    // mark named exports
    for (uint32_t i = 0; i < exports->typeCount; ++i) {
        struct SerializedObjectInfo* objectInfo = _oneGuiInsertType(state, chunkState, exports->typeExports[i].name, exports->typeExports[i].exportValue);
        objectInfo->serializedIndex = i;
    }

    // mark data exports
    for (uint32_t i = 0; i < exports->valueCount; ++i) {
        struct SerializedObjectInfo* objectInfo = _oneGuiAddReference(state, chunkState, exports->valueExports[i].name, exports->valueExports[i].exportValue, refGetDataType(exports->valueExports[i].exportValue));
        objectInfo->serializedIndex = i;
    }

    _oneGuiGenerateTypes(chunkState, output);

    _oneGuiGenerateData(chunkState, output);

    len = ogFileSeek(output, 0, SeekTypeCurr) - dataStart;
    ogFileSeek(output, dataStart - sizeof(uint64_t), SeekTypeSet);
    // write the correct file length
    ogFileWrite(output, &len, sizeof(uint64_t));
}

void oneGuiSerializeWithState(struct SerializerState* state, struct ModuleExports* exports, struct OGFile* output) {
    struct SerializedChunkState chunkState;
    chunkState.objectToObjectInfo = rangedBinaryTreeNew();
    chunkState.typeToObjectInfo = hashTableNew(hashTableBasicEquality, hashTableIntegerHash, HashTableFlagsRetainKey | HashTableFlagsRetainValue);

    _oneGuiSerializeWithState(state, &chunkState, exports, output);

    refRelease(chunkState.objectToObjectInfo);
    refRelease(chunkState.typeToObjectInfo);
}

void oneGuiSerializeExports(struct ModuleExports* exports, struct OGFile* output) {
    oneGuiSerializeWithState(gSerializerState, exports, output);
}