#include "onegui.h"

#include "../types/type_builder.h"
#include "../data_structures/hash_table.h"

void oneGuiInit() {
    typeBuilderInit();
    hashTableTypeInit();
}