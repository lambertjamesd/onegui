
#include "ranged_binary_tree.h"
#include "../types/type_builder.h"
#include "../ref/ref.h"
#include <assert.h>

struct DataType* _gRangeBinaryTreeType;
struct DataType* _gRangeBinaryTreeNodeType;

void rangeBinaryTreeInit() {
    _gRangeBinaryTreeNodeType = typeBuilderNewObject(sizeof(struct RangedBinaryTreeNode), 6);
    
    struct DataType* pointerToNode = typeBuilderNewPointerType(_gRangeBinaryTreeNodeType);

    TYPE_BUILDER_APPEND_SUB_TYPE(_gRangeBinaryTreeNodeType, struct RangedBinaryTreeNode, left, pointerToNode);
    TYPE_BUILDER_APPEND_SUB_TYPE(_gRangeBinaryTreeNodeType, struct RangedBinaryTreeNode, right, pointerToNode);
    TYPE_BUILDER_APPEND_SUB_TYPE(_gRangeBinaryTreeNodeType, struct RangedBinaryTreeNode, min, typeBuilderGetPrimitive(DataTypeUInt64));
    TYPE_BUILDER_APPEND_SUB_TYPE(_gRangeBinaryTreeNodeType, struct RangedBinaryTreeNode, len, typeBuilderGetPrimitive(DataTypeUInt32));
    TYPE_BUILDER_APPEND_SUB_TYPE(_gRangeBinaryTreeNodeType, struct RangedBinaryTreeNode, height, typeBuilderGetPrimitive(DataTypeUInt32));
    TYPE_BUILDER_APPEND_SUB_TYPE(_gRangeBinaryTreeNodeType, struct RangedBinaryTreeNode, value, typeBuilderGetPointerToUnknown());

    _gRangeBinaryTreeType = typeBuilderNewObject(sizeof(struct RangedBinaryTree), 1);

    TYPE_BUILDER_APPEND_SUB_TYPE(_gRangeBinaryTreeType, struct RangedBinaryTree, root, pointerToNode);
    TYPE_BUILDER_APPEND_SUB_TYPE(_gRangeBinaryTreeType, struct RangedBinaryTree, size, typeBuilderGetPrimitive(DataTypeUInt32));

    refRelease(pointerToNode);    
}

struct RangedBinaryTree* rangedBinaryTreeNew() {
    return refMalloc(_gRangeBinaryTreeType);
}

int rangeBinaryTreeNodeHeight(struct RangedBinaryTreeNode* node) {
    if (!node) {
        return 0;
    }

    return node->height;
}

void rangeBinaryTreeNodeRecalcHeight(struct RangedBinaryTreeNode* node) {
    if (!node) {
        return;
    }

    int left = rangeBinaryTreeNodeHeight(node->left);
    int right = rangeBinaryTreeNodeHeight(node->right);

    if (left > right) {
        node->height = left + 1;
    } else {
        node->height = right + 1;
    }
}

struct RangedBinaryTreeNode* rangedBinaryTreeRotateRight(struct RangedBinaryTreeNode* node) {
    struct RangedBinaryTreeNode* newRoot = node->left;
    
    node->left = newRoot->right;
    newRoot->right = node;

    rangeBinaryTreeNodeRecalcHeight(newRoot->left);
    rangeBinaryTreeNodeRecalcHeight(newRoot->right);
    rangeBinaryTreeNodeRecalcHeight(newRoot);
    
    return newRoot;
}

struct RangedBinaryTreeNode* rangedBinaryTreeRotateLeft(struct RangedBinaryTreeNode* node) {
    struct RangedBinaryTreeNode* newRoot = node->right;
    
    node->right = newRoot->left;
    newRoot->left = node;

    rangeBinaryTreeNodeRecalcHeight(newRoot->left);
    rangeBinaryTreeNodeRecalcHeight(newRoot->right);
    rangeBinaryTreeNodeRecalcHeight(newRoot);
    
    return newRoot;
}

struct RangedBinaryTreeNode* rangedBinaryTreeInsertNode(struct RangedBinaryTreeNode* node, struct RangedBinaryTreeNode* newNode) {
    if (!node) {
        return newNode;
    }

    if (node->min < newNode->min + (uint64_t)newNode->len && newNode->min < node->min + (uint64_t)node->len) {
        // no overlapping ranges allowed
        return NULL;
    }

    if (node->min > newNode->min) {
        struct RangedBinaryTreeNode* newLeft = rangedBinaryTreeInsertNode(node->left, newNode);
        if (!newLeft) {
            return NULL;
        }
        node->left = newLeft;
    } else {
        struct RangedBinaryTreeNode* newRight = rangedBinaryTreeInsertNode(node->right, newNode);
        if (!newRight) {
            return NULL;
        }
        node->right = newRight;
    }

    int leftHeight = rangeBinaryTreeNodeHeight(node->left);
    int rightHeight = rangeBinaryTreeNodeHeight(node->right);

    if (rightHeight > leftHeight) {
        node->height = rightHeight + 1;

        if (rightHeight - leftHeight >= 2) {
            return rangedBinaryTreeRotateLeft(node);
        }
    } else {
        node->height = leftHeight + 1;

        if (leftHeight - rightHeight >= 2) {
            return rangedBinaryTreeRotateRight(node);
        }
    }

    return node;
}

bool rangedBinaryTreeInsert(struct RangedBinaryTree* tree, uint64_t at, uint32_t len, void* value) {
    struct RangedBinaryTreeNode* newNode = refMalloc(_gRangeBinaryTreeNodeType);

    newNode->min = at;
    newNode->len = len;
    newNode->height = 1;
    newNode->value = refRetain(value);

    struct RangedBinaryTreeNode* newRoot = rangedBinaryTreeInsertNode(tree->root, newNode);

    if (!newRoot) {
        refRelease(newNode);
        return false;
    }

    tree->root = newRoot;
    tree->size++;

    return true;
}

bool rangedBinaryTreeNodeGet(struct RangedBinaryTreeNode* node, uint64_t at, struct RangedBinaryTreeNode** result) {
    if (!node) {
        return false;
    }

    if (at < node->min) {
        return rangedBinaryTreeNodeGet(node->left, at, result);
    }

    if (at >= node->min + (uint64_t)node->len) {
        return rangedBinaryTreeNodeGet(node->right, at, result);
    }

    if (result) {
        *result = node;
    }

    return true;
}

bool rangedBinaryTreeGet(struct RangedBinaryTree* tree, uint64_t at, struct RangedBinaryTreeNode** result) {
    return rangedBinaryTreeNodeGet(tree->root, at, result);
}

uint32_t rangedBinaryTreeSize(struct RangedBinaryTree* tree) {
    return tree->size;
}

void _rangedBinaryTreeIteratorFallLeft(struct RangedBinaryTreeIterator* iterator, struct RangedBinaryTreeNode* current) {
    while (current && current->left) {
        current = current->left;
        iterator->path &= ~(1 << iterator->depth);
        iterator->depth++;
    }

    iterator->current = current;
}

void rangedBinaryTreeIteratorInit(struct RangedBinaryTreeIterator* iterator, struct RangedBinaryTree* tree) {
    iterator->depth = 0;
    iterator->path = 0;
    iterator->tree = tree;
    iterator->current = NULL;
    _rangedBinaryTreeIteratorFallLeft(iterator, tree->root);
}

struct RangedBinaryTreeNode* rangedBinaryTreeIteratorCurrent(struct RangedBinaryTreeIterator* iterator) {
    return iterator->current;
}

bool _rangedBinaryTreeFindLastLeftTurn(struct RangedBinaryTreeIterator* iterator, struct RangedBinaryTreeNode* current, uint32_t depth) {
    uint32_t depthMask = 1 << depth;

    if (depth < iterator->depth) {
        bool subResult = _rangedBinaryTreeFindLastLeftTurn(iterator, (depthMask & iterator->path) ? current->right : current->left, depth + 1);

        if (subResult) {
            return subResult;
        }

        // if coming from the left, then the current
        // node becomes the next node
        if (!(depthMask & iterator->path)) {
            iterator->depth = depth;
            iterator->current = current;
            return true;
        }
    }

    return false;
}

void rangedBinaryTreeIteratorNext(struct RangedBinaryTreeIterator* iterator) {
    if (!iterator->current) {
        return;
    }

    if (iterator->current->right) {
        iterator->path |= (1 << iterator->depth);
        iterator->depth++;
        _rangedBinaryTreeIteratorFallLeft(iterator, iterator->current->right);
        return;
    }

    if (!_rangedBinaryTreeFindLastLeftTurn(iterator, iterator->tree->root, 0)) {
        iterator->tree = NULL;
        iterator->current = NULL;
        return;
    }
}