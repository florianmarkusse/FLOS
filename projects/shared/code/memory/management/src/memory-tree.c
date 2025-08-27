#include "shared/memory/management/memory-tree.h"

U32 getNodeFromTreeWithFreeList(U32_max_a *freeList, void_max_a *nodes) {
    if (freeList->len > 0) {
        U32 result = freeList->buf[freeList->len - 1];
        freeList->len--;
        return result;
    }

    if (nodes->len < nodes->cap) {
        return nodes->len;
    }

    return 0;
}
