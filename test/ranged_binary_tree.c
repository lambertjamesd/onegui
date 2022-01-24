#include "../src/data_structures/ranged_binary_tree.h"

#include "test.h"

#include "../src/ref/ref.h"
#include "../src/ref/ref_privite.h"
#include "../src/types/string.h"

void testRangedBinaryTree() {
    struct RangedBinaryTree* tree = rangedBinaryTreeNew();

    OString a = OSTR_NEW_FROM_CSTR("a");
    OString b = OSTR_NEW_FROM_CSTR("b");
    OString c = OSTR_NEW_FROM_CSTR("c");

    TEST_ASSERT(rangedBinaryTreeInsert(tree, 10, 10, a));

    struct RangedBinaryTreeNode* output;
    TEST_ASSERT(!rangedBinaryTreeGet(tree, 9, NULL));
    TEST_ASSERT(rangedBinaryTreeGet(tree, 10, &output));
    TEST_ASSERT(output->value == a);
    TEST_ASSERT(rangedBinaryTreeGet(tree, 19, NULL));
    TEST_ASSERT(!rangedBinaryTreeGet(tree, 20, NULL));
    TEST_ASSERT(tree->root == output);

    // should not insert if the new range
    // intersect an existing range
    TEST_ASSERT(!rangedBinaryTreeInsert(tree, 19, 10, b));
    TEST_ASSERT(!rangedBinaryTreeGet(tree, 20, NULL));
    TEST_ASSERT(rangedBinaryTreeInsert(tree, 20, 10, b));
    TEST_ASSERT(rangedBinaryTreeGet(tree, 20, &output));
    TEST_ASSERT(output->value == b);

    TEST_ASSERT(rangedBinaryTreeInsert(tree, 30, 10, c));

    TEST_ASSERT(rangedBinaryTreeGet(tree, 10, &output));
    TEST_ASSERT(output->value == a);
    TEST_ASSERT(rangedBinaryTreeGet(tree, 20, &output));
    TEST_ASSERT(output->value == b);
    // verify the tree rotated correctly
    TEST_ASSERT(tree->root == output);
    TEST_ASSERT(rangedBinaryTreeGet(tree, 30, &output));
    TEST_ASSERT(output->value == c);

    TEST_ASSERT(_refGetCount(a) == 2);
    TEST_ASSERT(_refGetCount(b) == 2);
    TEST_ASSERT(_refGetCount(c) == 2);

    refRelease(tree);

    TEST_ASSERT(_refGetCount(a) == 1);
    TEST_ASSERT(_refGetCount(b) == 1);
    TEST_ASSERT(_refGetCount(c) == 1);

    refRelease(a);
    refRelease(b);
    refRelease(c);
}