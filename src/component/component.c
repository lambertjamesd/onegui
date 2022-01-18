
#include "component.h"

#include "../ref/ref.h"

#include <memory.h>
#include <malloc.h>

struct Component* newComponent(struct ComponentTemplate* template) {
    struct Component* result = malloc(sizeof(struct Component));

    size_t internalStateSize = dataTypeSize(template->internalStateType);

    result->internalState = refMalloc(template->internalStateType);
    // TODO handle reference count updates
    memcpy(result->internalState, template->internalState, internalStateSize);
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