#include "serialize.h"

#include <stdint.h>

struct SerializedObjectInfo {
    uint32_t serializedIndex;
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
    struct RangedBinaryTreeNode* objectToObjectInfo;
    struct DeferredWeakLinkArray* deferredWeakLinks;
};

void _oneGuiInsertType(struct SerializedChunkState* chunkState, struct DataType* type) {
    if (!type || hashTableGet(chunkState->typeToIndexMapping, (uint64_t)type, NULL)) {
        return;
    }

    hashTableSet(chunkState->typeToIndexMapping, (uint64_t)type, (uint64_t)hashTableSize(chunkState->typeToIndexMapping));

    switch (type->type) {
        case DataTypePointer:
            _oneGuiInsertType(chunkState, ((struct PointerDataType*)type)->subType);
            break;
        case DataTypeFixedArray:
            _oneGuiInsertType(chunkState, ((struct FixedArrayDataType*)type)->subType);
            break;
        case DataTypeDynamicArray:
            _oneGuiInsertType(chunkState, ((struct DynamicArrayDataType*)type)->subType);
            break;
        case DataTypeObject:
        {
            struct ObjectDataType* objectType = (struct ObjectDataType*)type;

            for (unsigned i = 0; i < objectType->objectSubTypes->header.count; ++i) {
                _oneGuiInsertType(chunkState, objectType->objectSubTypes->elements[i].type);
            }
            break;
        }
    }
}

void _oneGuiPopulateTypes(struct SerializerState* state, struct SerializedChunkState* chunkState, void* ref, struct DataType* type) {
    _oneGuiInsertType(chunkState, type);

    // TODO keep track of visited nodes

    switch (type->type) {
        case DataTypePointer:
            _oneGuiPopulateTypes(state, chunkState, *((void**)ref), ((struct PointerDataType*)type)->subType);
            break;
        case DataTypeFixedArray:
        {
            // struct FixedArrayDataType* arrayType = (struct FixedArrayDataType*)type;
            // unsigned elementSize = dataTypeSize(arrayType->subType);

            break;
        }
    }
}

void _oneGuiSerializeWithState(struct SerializerState* state, struct SerializedChunkState* chunkState, void* ref, struct FileInterface* fileInterface, void* file) {

}

void oneGuiSerializeWithState(struct SerializerState* state, void* ref, struct FileInterface* fileInterface, void* file) {

}