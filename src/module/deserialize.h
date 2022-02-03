#ifndef __ONEGUI_DESERIALIZE_H__
#define __ONEGUI_DESERIALIZE_H__

#include "exports.h"
#include <stdbool.h>
#include "../file/file.h"

bool oneGuiDeserializeWithState(struct SerializerState* state, struct OGFile* input, struct ModuleExports** output);
bool oneGuiDeserialize(struct SerializerState* state, struct OGFile* input, struct ModuleExports** output);

#endif