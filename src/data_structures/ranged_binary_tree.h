#ifndef __ONEGUI_RANGED_BINARY_TREE_H__
#define __ONEGUI_RANGED_BINARY_TREE_H__

#include <stdint.h>
#include <stdbool.h>

struct RangedBinaryTreeNode {
    struct RangedBinaryTreeNode* left;
    struct RangedBinaryTreeNode* right;
    uint64_t min;
    uint32_t len;
    uint32_t height;
    void* value;
};

struct RangedBinaryTree {
    struct RangedBinaryTreeNode* root;
    uint32_t size;
};

void rangeBinaryTreeInit();

struct RangedBinaryTree* rangedBinaryTreeNew();
bool rangedBinaryTreeInsert(struct RangedBinaryTree* tree, uint64_t at, uint32_t len, void* value);
bool rangedBinaryTreeGet(struct RangedBinaryTree* tree, uint64_t at, struct RangedBinaryTreeNode** result);
uint32_t rangedBinaryTreeSize(struct RangedBinaryTree* tree);


#endif