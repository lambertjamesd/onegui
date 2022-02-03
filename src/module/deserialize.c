#include "deserialize.h"
#include "../error/error.h"
#include "../types/string.h"

struct OGDeserializeState {
    struct NamedExportArray* types;
    struct NamedExportArray* values;
};

OString _oneGuiReadString(struct OGFile* input) {
    int len;
    uint8_t curr;
    int lengthBytes = 0;

    do {
        ogFileRead(input, &curr, 1);
        len = (len << 7) | (curr & 0x7F);
        ++lengthBytes;
    } while ((curr & 0x80) != 0 && lengthBytes < 6);

    if (lengthBytes == 6) {
        ogReportError("_oneGuiReadString: String too long");
        return NULL;
    }

    OString result = ostrNewFromLen(len);
    ogFileRead(input, result + lengthBytes, len);
    return result;
}

bool oneGuiDeserializeWithState(struct SerializerState* state, struct OGFile* input, struct ModuleExports** output) {
    uint16_t version;
    ogFileRead(input, &version, sizeof(uint16_t));

    if (version != 0) {
        ogReportError("oneGuiDeserializeWithState: Invalid file version");
        return false;
    }

    uint32_t importCount;
    uint32_t dataCount;

    ogFileRead(input, &importCount, sizeof(uint32_t));
    ogFileRead(input, &dataCount, sizeof(uint32_t));

    return true;
}

bool oneGuiDeserialize(struct SerializerState* state, struct OGFile* input, struct ModuleExports** output) {
    return oneGuiDeserializeWithState(gSerializerState, input, output);
}