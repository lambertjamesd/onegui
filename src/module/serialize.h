#ifndef __ONEGUE_SERIALIZE_H__
#define __ONEGUE_SERIALIZE_H__

#include <stdint.h>
#include "exports.h"
#include "../file/file.h"

#define ONEGUI_DATA_HEADER  0x41544144

void oneGuiSerializeInit();

void oneGuiSerializeWithState(struct SerializerState* state, struct ModuleExports* exports, struct OGFile* output);

void oneGuiSerializeExports(struct ModuleExports* exports, struct OGFile* output);

#endif