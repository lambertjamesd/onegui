#ifndef __ONEGUI_FILE_H__
#define __ONEGUI_FILE_H__

#include <stdint.h>

enum SeekType {
    SeekTypeSet,
    SeekTypeCurr,
    SeekTypeEnd,
};

typedef int (*FileWrite)(void* file, const char* data, int count);
typedef int (*FileRead)(void* file, char* data, int count);
typedef int64_t (*FileSeek)(void* file, int64_t offset, enum SeekType seekType);
typedef void (*FileClose)(void* file);

struct FileInterface {
    FileWrite write;
    FileRead read;
    FileSeek seek;
    FileClose close;
};

struct OGFile {
    void* file;
    struct FileInterface* interface;
};

int ogFileRead(struct OGFile* file, void* data, int bytes);
int ogFileWrite(struct OGFile* file, const void* data, int bytes);
int64_t ogFileSeek(struct OGFile* file, int64_t offset, enum SeekType seekType);
void ogFileClose(struct OGFile* file);

#endif