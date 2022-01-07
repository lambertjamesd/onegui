#ifndef __ONEGUI_COMPONENT_TEMPLATE_H__
#define __ONEGUI_COMPONENT_TEMPLATE_H__

#include "../types/types.h"
#include "module.h"

enum ComponentTemplateFlags {
    ComponentTemplateFlagsCanFlexWidth = (1 << 0),
    ComponentTemplateFlagsCanFlexHeight = (1 << 1),
};

struct Component;
typedef void (*LayoutComponent)(struct Component* component);
typedef void (*InitComponent)(struct Component* component);

struct ComponentTemplate {
    InitComponent initCallback;
    LayoutComponent layoutCallback;
    enum ComponentTemplateFlags flags;
    struct AnyObjectReference internalState;
    struct Module* forModule;
};

#endif