#include "file.h"


int ogFileRead(struct OGFile* file, void* data, int bytes) {
    return file->interface->read(file->file, data, bytes);
}

int ogFileWrite(struct OGFile* file, void* data, int bytes) {
    return file->interface->write(file->file, data, bytes);
}

int64_t ogFileSeek(struct OGFile* file, int64_t offset, enum SeekType seekType) {
    return file->interface->seek(file->file, offset, seekType);
}