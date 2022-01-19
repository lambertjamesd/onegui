#include "string.h"

const char* _ostrLen(const char* input, int* output) {
    int result = *input & 0x7F;

    while ((*input) & 0x80) {
        result = (result << 7) | (*input & 0x7F);
        ++input;
    }

    ++input;

    if (output) {
        *output = result;
    }

    return input;
}

int ostrLen(const char* ostr) {
    int result;
    _ostrLen(ostr, &result);
    return result;
}

bool ostrEqual(const char* a, const char* b) {
    int aLen;
    int bLen;

    a = _ostrLen(a, &aLen);
    b = _ostrLen(b, &bLen);

    if (aLen != bLen) {
        return false;
    }

    for (int i = 0; i < aLen; ++i) {
        if (a[i] != b[i]) {
            return false;
        }
    }

    return true;
}

bool ostrEqualCStr(const char* ostr, const char* cstr) {
    int len;

    ostr = _ostrLen(ostr, &len);

    for (int i = 0; i < len; ++i) {
        if (!*cstr || ostr[i] != cstr[i]) {
            return false;
        }
    }

    return cstr[len] == '\0';
}