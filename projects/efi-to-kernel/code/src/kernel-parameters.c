#include "efi-to-kernel/kernel-parameters.h"

void setPackedMemoryAllocator(PackedNodeAllocator *packedNodeAllocator,
                              void **packedTree, NodeAllocator *nodeAllocator,
                              void *tree) {
    *packedNodeAllocator = (PackedNodeAllocator){
        .nodes = (PackedVoid_max_a){.buf = nodeAllocator->nodes.buf,
                                    .len = nodeAllocator->nodes.len,
                                    .cap = nodeAllocator->nodes.cap},
        .nodesFreeList =
            (PackedVoidPtr_max_a){.buf = nodeAllocator->nodesFreeList.buf,
                                  .cap = nodeAllocator->nodesFreeList.cap,
                                  .len = nodeAllocator->nodesFreeList.len},
        .elementSizeBytes = nodeAllocator->elementSizeBytes};
    *packedTree = tree;
}
