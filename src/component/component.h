#ifndef __ONEGUI_COMPONENT_H__
#define __ONEGUI_COMPONENT_H__

#include "template.h"

enum ComponentFlags {
    ComponentFlagsDirtyLayout = (1 << 0),
    ComponentFlagsCalcWidth = (1 << 1),
    ComponentFlagsCalcHeight = (1 << 2),
};

struct Component {
    void* internalState;
    void* prevInternalState;
    enum ComponentFlags flags;
    struct ComponentTemplate* template;
};

struct Component* componentNew(struct ComponentTemplate* template);

void componentInit(struct Component* component);

void componentLayout(struct Component* component);

#endif