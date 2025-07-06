#include "efi-to-kernel/kernel-parameters.h"

void setPackedMemoryAllocator(PackedMemoryAllocator *packedMemoryAllocator,
                              RedBlackNodeMM_max_a *nodes, RedBlackNodeMM *root,
                              RedBlackNodeMMPtr_max_a *freeList) {
    *packedMemoryAllocator = (PackedMemoryAllocator){
        .nodes = (PackedRedBlackNodeMM_max_a){.buf = nodes->buf,
                                              .len = nodes->len,
                                              .cap = nodes->cap},
        .freeList = (PackedRedBlackNodeMMPtr_max_a){.buf = freeList->buf,
                                                    .cap = freeList->cap,
                                                    .len = freeList->len},
        .tree = root};
}
