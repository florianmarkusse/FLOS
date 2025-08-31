#include "shared/memory/management/memory-tree.h"

U32 getNodeFromTreeWithFreeList(TreeWithFreeList *treeWithFreeList) {
    if (treeWithFreeList->freeList.len > 0) {
        U32 result =
            treeWithFreeList->freeList.buf[treeWithFreeList->freeList.len - 1];
        treeWithFreeList->freeList.len--;
        return result;
    }

    if (treeWithFreeList->len < treeWithFreeList->cap) {
        U32 result = treeWithFreeList->len;
        treeWithFreeList->len++;
        return result;
    }

    return 0;
}
