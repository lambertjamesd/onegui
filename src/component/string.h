#ifndef __ONEGUI_STRING_H__
#define __ONEGUI_STRING_H__

struct String {
    unsigned int byteLength;
    // utf-8 encoded
    const char* data;
};

#endif