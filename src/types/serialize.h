#ifndef __ONEGUE_SERIALIZE_H__
#define __ONEGUE_SERIALIZE_H__

#include <stdint.h>
#include "../data_structures/hash_table.h"
#include "../data_structures/ranged_binary_tree.h"

enum SeekType {
    SeekTypeSet,
    SeekTypeCurr,
    SeekTypeEnd,
};

// TODO move this to a seperate file
typedef int (*FileWrite)(void* file, const char* data, int count);
typedef int (*FileRead)(void* file, char* data, int count);
typedef int64_t (*FileSeek)(void* file, int64_t offset, enum SeekType seekType);

struct FileInterface {
    FileWrite write;
    FileRead read;
    FileSeek seek;
};

struct OGFile {
    void* file;
    struct FileInterface* interface;
};

struct ObjectExportInformation {
    void* objectRef;
    OString moduleName;
    OString name;
};

struct SerializerState {
    struct RangedBinaryTree* addressToObject;
    struct HashTable* objectNameToObject;
};

void oneGuiSerializeWithState(struct SerializerState* state, struct RangedBinaryTree* namedExports, struct OGFile output);

void oneGuiSerializeExports(struct RangedBinaryTree* namedExports, struct OGFile output);

#endif