#include "string.h"

OString ostrNewFromCStr(const char* cstr) {
    return refMallocString(typeBuilderGetStringType(), strlen(cstr), cstr);
}

const char* ostrToCStr(ConstOString input, int* output) {
    if (!input) {
        *output = 0;
        return "";
    }

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

int ostrLen(ConstOString ostr) {
    int result;
    ostrToCStr(ostr, &result);
    return result;
}

bool ostrEqual(ConstOString a, ConstOString b) {
    int aLen;
    int bLen;

    a = ostrToCStr(a, &aLen);
    b = ostrToCStr(b, &bLen);

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

int ostrCompare(ConstOString a, ConstOString b) {
    int aLen;
    int bLen;

    a = ostrToCStr(a, &aLen);
    b = ostrToCStr(b, &bLen);

    for (int i = 0; i < aLen && i < bLen; ++i) {
        int diff = a[i] - b[i];

        if (diff != 0) {
            return diff;
        }
    }

    return aLen - bLen;
}

bool ostrEqualCStr(ConstOString ostr, ConstOString cstr) {
    int len;

    ostr = ostrToCStr(ostr, &len);

    for (int i = 0; i < len; ++i) {
        if (!*cstr || ostr[i] != cstr[i]) {
            return false;
        }
    }

    return cstr[len] == '\0';
}