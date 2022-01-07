
#include "component.h"

#include <memory.h>

struct Component* newComponent(struct ComponentTemplate* template) {
    struct Component* result = malloc(sizeof(struct Component));

    size_t internalStateSize = dataTypeSize(template->internalState.dataType);

    result->internalState = malloc(internalStateSize);
    memcpy(result->internalState, template->internalState.data, internalStateSize);
    result->flags = ComponentFlagsDirtyLayout;
    result->template = template;

    return result;
}

void componentInit(struct Component* component) {
    component->template->initCallback(component);
}

void componentLayout(struct Component* component) {
    if (component->flags & ComponentFlagsDirtyLayout) {
        component->flags &= ~ComponentFlagsDirtyLayout;
        component->template->layoutCallback(component);
    }
}