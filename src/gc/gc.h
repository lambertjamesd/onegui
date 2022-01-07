#ifndef __ONEGUI_GC__
#define __ONEGUI_GC__

#include "../types/types.h"

void gc_init(unsigned initialSize);
GCRef gc_malloc(GCRef typeEntry);
GCRef gc_malloc_array(GCRef arrayType, unsigned count);
void* gc_lookup(GCRef entry);
void gc_collect();

void gc_pin(GCRef ref);
void gc_unpin(GCRef ref);

#endif