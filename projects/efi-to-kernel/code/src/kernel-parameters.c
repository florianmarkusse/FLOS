#include "efi-to-kernel/kernel-parameters.h"

void setPackedMemoryAllocator(PackedTreeWithFreeList *packedTreeWithFreeList,
                              TreeWithFreeList *treeWithFreeList) {
    *packedTreeWithFreeList = (PackedTreeWithFreeList){
        .nodes = (PackedVoid_max_a){.buf = treeWithFreeList->nodes.buf,
                                    .len = treeWithFreeList->nodes.len,
                                    .cap = treeWithFreeList->nodes.cap},
        .freeList =
            (PackedVoidPtr_max_a){.buf = treeWithFreeList->freeList.buf,
                                  .cap = treeWithFreeList->freeList.cap,
                                  .len = treeWithFreeList->freeList.len},
        .tree = treeWithFreeList->tree};
}
