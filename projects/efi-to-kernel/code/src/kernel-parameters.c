#include "efi-to-kernel/kernel-parameters.h"

void setPackedMemoryAllocator(PackedTreeWithFreeList *packedTreeWithFreeList,
                              TreeWithFreeList *treeWithFreeList) {
    *packedTreeWithFreeList = (PackedTreeWithFreeList){
        .freeList = {.buf = treeWithFreeList->freeList.buf,
                     .cap = treeWithFreeList->freeList.cap,
                     .len = treeWithFreeList->freeList.len},
        .buf = treeWithFreeList->buf,
        .len = treeWithFreeList->len,
        .cap = treeWithFreeList->cap,
        .elementSizeBytes = treeWithFreeList->elementSizeBytes,
        .rootIndex = treeWithFreeList->rootIndex};
}
