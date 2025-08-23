#include "shared/memory/management/memory-tree.h"

void *getNodeFromTreeWithFreeList(voidPtr_max_a *freeList, void_max_a *nodes,
                                  U32 elementSize) {
    if (freeList->len > 0) {
        void *result = freeList->buf[freeList->len - 1];
        freeList->len--;
        return result;
    }

    if (nodes->len < nodes->cap) {
        U8 *base = nodes->buf;
        void *result = base + (elementSize * nodes->len);
        nodes->len++;
        return result;
    }

    return nullptr;
}
