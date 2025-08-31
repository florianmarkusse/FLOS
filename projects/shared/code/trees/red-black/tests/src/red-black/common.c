#include "shared/trees/red-black/tests/red-black/common.h"
#include "posix/test-framework/test.h"
#include "shared/trees/red-black/common.h"
#include "shared/trees/red-black/tests/assert.h"

void *getFromNodes(TreeWithFreeList *treeWithFreeList) {
    if (treeWithFreeList->freeList.len) {
        U32 freeIndex =
            treeWithFreeList->freeList.buf[treeWithFreeList->freeList.len - 1];
        treeWithFreeList->freeList.len--;

        return getNode(treeWithFreeList, freeIndex);
    }
    if (treeWithFreeList->len == treeWithFreeList->cap) {
        TEST_FAILURE {
            INFO(STRING("Tree contains too many nodes to fit in nodes array."
                        "Increase max size or decrease nodes "
                        "in Red-Black tree. Current maximum size: "));
            INFO(MAX_NODES_IN_TREE, .flags = NEWLINE);
        }
    }

    void *result = ((U8 *)treeWithFreeList->buf) +
                   (treeWithFreeList->elementSizeBytes * treeWithFreeList->len);
    treeWithFreeList->len++;
    return result;
}
