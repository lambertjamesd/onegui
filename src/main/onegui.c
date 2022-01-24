#include "onegui.h"

#include "../types/type_builder.h"
#include "../data_structures/hash_table.h"
#include "../data_structures/ranged_binary_tree.h"
#include "../types/copy.h"

void oneGuiInit() {
    typeBuilderInit();
    hashTableTypeInit();
    rangeBinaryTreeInit();
    oneGuiCopyInit();
}