#ifndef __ONEGUI_COPY_H__
#define __ONEGUI_COPY_H__

#include "../data_structures/ranged_binary_tree.h"

struct OGDeepCopyState;

void oneGuiCopyInit();
void* oneGuiDeepCopyWithState(const void* ref, struct OGDeepCopyState* state);
void* oneGuiDeepCopy(const void* ref);

#endif