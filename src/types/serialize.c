#include "serialize.h"

#include <stdint.h>
#include "../types/type_builder.h"

struct DataType* _gSerializedObjectInfoType;

enum SerializedObjectInfoFlags {
    SerializedObjectInfoFlagsIsSerialized = (1 << 0),
};

struct SerializedObjectInfo {
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
    struct HashTable* typeToIndexMapping;
    struct HashTable* usedTypeImports;
    struct HashTable* usedImports;
    struct RangedBinaryTree* objectToObjectInfo;
};

void serializeInit() {
    _gSerializedObjectInfoType = typeBuilderNewObject(sizeof(struct SerializedObjectInfo), 2);
    TYPE_BUILDER_APPEND_SUB_TYPE(_gSerializedObjectInfoType, struct SerializedObjectInfo, serializedIndex, typeBuilderGetPrimitive(DataTypeUInt32));
    TYPE_BUILDER_APPEND_SUB_TYPE(_gSerializedObjectInfoType, struct SerializedObjectInfo, flags, typeBuilderGetPrimitive(DataTypeUInt32));
}

void _oneGuiInsertType(struct SerializerState* state, struct SerializedChunkState* chunkState, struct DataType* type) {
    if (hashTableGetK(chunkState->usedTypeImports, type, NULL)) {
        return;
    }

    struct RangedBinaryTreeNode* namedType;
    // check to see if the object type is exported from another module
    if (rangedBinaryTreeGet(state->addressToObject, (uint64_t)type, &namedType)) {
        hashTableSetKV(chunkState->usedTypeImports, type, namedType->value);
        return;
    }

    if (!type || hashTableGet(chunkState->typeToIndexMapping, (uint64_t)type, NULL)) {
        return;
    }

    hashTableSet(chunkState->typeToIndexMapping, (uint64_t)type, (uint64_t)hashTableSize(chunkState->typeToIndexMapping) + 1);

    if (!(type->flags & (DataTypeFlagsHasStrongRef | DataTypeFlagsHasWeakRef))) {
        return;
    }

    switch (type->type) {
        case DataTypePointer:
            _oneGuiInsertType(state, chunkState, ((struct PointerDataType*)type)->subType);
            break;
        case DataTypeWeakPointer:
            _oneGuiInsertType(state, chunkState, ((struct PointerDataType*)type)->subType);
            break;
        case DataTypeFixedArray:
            _oneGuiInsertType(state, chunkState, ((struct FixedArrayDataType*)type)->subType);
            break;
        case DataTypeDynamicArray:
            _oneGuiInsertType(state, chunkState, ((struct DynamicArrayDataType*)type)->subType);
            break;
        case DataTypeObject:
        {
            struct ObjectDataType* objectType = (struct ObjectDataType*)type;

            for (unsigned i = 0; i < objectType->objectSubTypes->header.count; ++i) {
                _oneGuiInsertType(state, chunkState, objectType->objectSubTypes->elements[i].type);
            }
            break;
        }
    }
}

void _oneGuiPopulateTypes(struct SerializerState* state, struct SerializedChunkState* chunkState, void* ref, struct DataType* type) {
    if (rangedBinaryTreeGet(state->addressToObject, (uint64_t)ref, NULL)) {
        return;
    }

    if (!ref || rangedBinaryTreeGet(chunkState->objectToObjectInfo, (uint64_t)ref, NULL)) {
        return;
    }

    struct SerializedObjectInfo* objectInfo = refMalloc(_gSerializedObjectInfoType);
    objectInfo->serializedIndex = rangedBinaryTreeSize(chunkState->objectToObjectInfo) + 1;

    if (!rangedBinaryTreeGet(state->addressToObject, (uint64_t)type, NULL)) {
        _oneGuiInsertType(state, chunkState, type);
    }

    if (!(type->flags & DataTypeFlagsHasStrongRef)) {
        return;
    }

    switch (type->type) {
        case DataTypePointer:
            _oneGuiPopulateTypes(state, chunkState, *((void**)ref), ((struct PointerDataType*)type)->subType);
            break;
        case DataTypeFixedArray:
        {
            struct FixedArrayDataType* arrayType = (struct FixedArrayDataType*)type;
            unsigned elementSize = dataTypeSize(arrayType->subType);

            char* curr = ref;

            for (unsigned i = 0; i < arrayType->elementCount; ++i) {
                _oneGuiPopulateTypes(state, chunkState, curr, arrayType->subType);
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
                _oneGuiPopulateTypes(state, chunkState, curr, arrayType->subType);
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
                    _oneGuiPopulateTypes(state, chunkState, (char*)ref + subType->offset, subType->type);
                }
            }

            break;
        }
    }
}

void _oneGuiSerializeWithState(struct SerializerState* state, struct SerializedChunkState* chunkState, struct FileInterface* fileInterface, void* file) {

}

void oneGuiSerializeWithState(struct SerializerState* state, struct RangedBinaryTree* namedExports, struct OGFile output) {

}