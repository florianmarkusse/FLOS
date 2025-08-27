#include "efi-to-kernel/kernel-parameters.h"

void setPackedMemoryAllocator(PackedTreeWithFreeList *packedTreeWithFreeList,
                              TreeWithFreeList *treeWithFreeList) {
    *packedTreeWithFreeList = (PackedTreeWithFreeList){
        .nodes = (Packedvoid_max_a){.buf = treeWithFreeList->nodes.buf,
                                    .len = treeWithFreeList->nodes.len,
                                    .cap = treeWithFreeList->nodes.cap},
        .freeList = {.buf = treeWithFreeList->freeList.buf,
                     .cap = treeWithFreeList->freeList.cap,
                     .len = treeWithFreeList->freeList.len},
        .tree = treeWithFreeList->tree,
        .nodeLocation = {.base = treeWithFreeList->nodeLocation.base,
                         .elementSizeBytes =
                             treeWithFreeList->nodeLocation.elementSizeBytes}};
}
