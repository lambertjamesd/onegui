#include "copy.h"

#include "../ref/ref.h"
#include "../types/type_builder.h"
#include <string.h>

struct OGPendingWeakPointers {
    struct DynamicArrayHeader header;
    void** pending[];
};

struct OGDeepCopyState {
    struct RangedBinaryTree* typeMapping;
    struct OGPendingWeakPointers* weakPointers;
};

struct DynamicArrayDataType *_gOGPendingWeakPointersType;

#define INTIAL_WEAK_CAPACITY    8

void oneGuiCopyInit() {
    _gOGPendingWeakPointersType = typeBuilderNewVariableArray(typeBuilderGetWeakPointerToUnknown());
}

void oneGuiPendWeakPointer(struct OGDeepCopyState* state, void** pointerToPointer) {
    if (!state->weakPointers) {
        state->weakPointers = (struct OGPendingWeakPointers*)refMallocArray(_gOGPendingWeakPointersType, INTIAL_WEAK_CAPACITY);
    } else if (state->weakPointers->header.count == state->weakPointers->header.capacity) {
        struct OGPendingWeakPointers* next = (struct OGPendingWeakPointers*)refMallocArray(_gOGPendingWeakPointersType, state->weakPointers->header.capacity * 2);

        for (unsigned i = 0; i < state->weakPointers->header.capacity; ++i) {
            next->pending[i] = state->weakPointers->pending[i];
        }

        refRelease(state->weakPointers);
        state->weakPointers = next;
    }

    state->weakPointers->pending[state->weakPointers->header.count++] = pointerToPointer;
}

bool oneGuiDeepCopyContent(const void* src, void* dest, struct DataType* type, struct OGDeepCopyState* state);

bool oneGuiDeepCopyObjectContent(const void* src, void* dest, struct ObjectDataType* type, struct OGDeepCopyState* state) {
    for (unsigned i = 0; i < type->objectSubTypes->header.count; ++i) {
        struct ObjectSubType* subtype = &type->objectSubTypes->elements[i];

        if (!oneGuiDeepCopyContent((const char*)src + subtype->offset, (char*)dest + subtype->offset, subtype->type, state)) {
            return false;
        }
    }

    return true;
}

bool oneGuiDeepCopyArray(const void* src, void* dest, struct DataType* elementType, unsigned count, struct OGDeepCopyState* state) {
    const char* srcIterator = src;
    char* destIterator = dest;
    size_t elementSize = dataTypeSize(elementType);

    for (unsigned i = 0; i < count; ++i) {
        if (!oneGuiDeepCopyContent(srcIterator, destIterator, elementType, state)) {
            return false;
        }
        srcIterator += elementSize;
        destIterator += elementSize;
    }

    return true;
}

bool oneGuiDeepCopyContent(const void* src, void* dest, struct DataType* type, struct OGDeepCopyState* state) {
    switch ((enum DataTypeType)type->type) {
        case DataTypeObject:
            return oneGuiDeepCopyObjectContent(src, dest, (struct ObjectDataType*)type, state);
        case DataTypeFixedArray:
        {
            struct FixedArrayDataType* arrayType = (struct FixedArrayDataType*)type;
            return oneGuiDeepCopyArray(src, dest, arrayType->subType, arrayType->elementCount, state);
        }
        case DataTypeVariableArray:
        {
            struct DynamicArrayDataType* arrayType = (struct DynamicArrayDataType*)type;
            struct DynamicArray* srcArray = (struct DynamicArray*)src;
            struct DynamicArray* destArray = (struct DynamicArray*)dest;

            destArray->header.count = srcArray->header.count;

            return oneGuiDeepCopyArray(srcArray->data, destArray->data, arrayType->subType, srcArray->header.count, state);
        }
        case DataTypeString:
            // strings aren't mutable and therefore aren't copied
            *((OString*)dest) = refRetain(*((OString*)src));
            break;
        case DataTypePointer:
            *((void**)dest) = refRetain(oneGuiDeepCopyWithState(*((void**)src), state));
            break;
        case DataTypeWeakPointer:
            {
                struct RangedBinaryTreeNode* existing;
                void* srcContent = *((void**)src);

                if (rangedBinaryTreeGet(state->typeMapping, (uint64_t)srcContent, &existing)) {
                    *((void**)dest) = existing->value + ((char*)srcContent - (char*)existing->min);
                } else {
                    oneGuiPendWeakPointer(state, (void**)dest);
                    *((void**)dest) = srcContent;
                }
            }
            break;
        case DataTypeUnknown:
        case DataTypeNull:
            break;
        case DataTypeInt8:
        case DataTypeUInt8:
            *((uint8_t*)dest) = *((const uint8_t*)src);
            break;
        case DataTypeInt16:
        case DataTypeUInt16:
            *((uint16_t*)dest) = *((const uint16_t*)src);
            break;
        case DataTypeInt32:
        case DataTypeUInt32:
        case DataTypeFloat32:
            *((uint32_t*)dest) = *((const uint32_t*)src);
            break;
        case DataTypeInt64:
        case DataTypeUInt64:
        case DataTypeFloat64:
            *((uint64_t*)dest) = *((const uint64_t*)src);
            break;
        case DataTypeOpaque:
        case DataTypeFunction:
            *((void**)dest) = *((void**)src);
            break;
    }

    return false;
}

void* oneGuiDeepCopyWithState(const void* ref, struct OGDeepCopyState* state) {
    if (!ref) {
        return NULL;
    }

    struct RangedBinaryTreeNode* existing;

    if (rangedBinaryTreeGet(state->typeMapping, (uint64_t)ref, &existing)) {
        return existing->value;
    }

    struct DataType* type = refGetDataType(ref);

    if (!type) {
        return NULL;
    }

    // strings aren't mutable and shouldn't be copied
    if (type->type == DataTypeString) {
        return (void*)ref;
    }

    void* result;
    uint32_t size;

    if (type->type == DataTypeVariableArray) {
        struct DynamicArrayDataType* arrayType = (struct DynamicArrayDataType*)type;
        struct DynamicArray* srcArray = (struct DynamicArray*)ref;
        result = refMallocArray(arrayType, srcArray->header.capacity);
        size = sizeof(struct DynamicArrayHeader) + dataTypeSize(arrayType->subType) * srcArray->header.capacity;
    } else {
        result = refMalloc(type);
        size = dataTypeSize(type);
    }

    bool didInsert = rangedBinaryTreeInsert(state->typeMapping, (uint64_t)ref, size, result);
    refRelease(result);
    // if the reference mapping failed to insert
    // then there are mulitple objects that overlap
    // in memory and that indicates something is really
    // wrong
    if (!didInsert) {
        return NULL;
    }

    if (!oneGuiDeepCopyContent(ref, result, type, state)) {
        return NULL;
    }

    return result;
}

void oneGuiDeepCopyUpdateWeakPointers(struct OGDeepCopyState* state) {
    if (!state->weakPointers) {
        return;
    }

    for (unsigned i = 0; i < state->weakPointers->header.count; ++i) {
        void* existingPointer = *state->weakPointers->pending[i];

        struct RangedBinaryTreeNode* existing;

        if (rangedBinaryTreeGet(state->typeMapping, (uint64_t)existingPointer, &existing)) {
            *state->weakPointers->pending[i] = existing->value + ((char*)existingPointer - (char*)existing->min);
        }
    }
}

void* oneGuiDeepCopy(const void* ref) {
    if (!ref) {
        return NULL;
    }

    struct OGDeepCopyState state;
    state.typeMapping = rangedBinaryTreeNew();
    state.weakPointers = NULL;
    void* result = oneGuiDeepCopyWithState(ref, &state);
    refRetain(result);
    refRelease(state.typeMapping);
    refRelease(state.weakPointers);
    return result;
}