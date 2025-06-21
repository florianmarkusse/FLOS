#include "efi-to-kernel/kernel-parameters.h"

void setPackedMemoryAllocator(PackedMemoryAllocator *packedMemoryAllocator,
                              Arena *arena, RedBlackNodeMM *root,
                              RedBlackNodeMMPtr_a *freeList) {
    *packedMemoryAllocator = (PackedMemoryAllocator){
        .freeList = (PackedRedBlackNodeMMPtr_a){.buf = freeList->buf,
                                                .len = freeList->len},
        .allocator = (PackedArena){.beg = arena->beg,
                                   .curFree = arena->curFree,
                                   .end = arena->end},
        .tree = root};
}
