#ifndef EFI_TO_KERNEL_KERNEL_PARAMETERS_H
#define EFI_TO_KERNEL_KERNEL_PARAMETERS_H

#include "shared/memory/allocator/arena.h"
#include "shared/memory/management/definitions.h"
#include "shared/trees/red-black/memory-manager.h"
#include "shared/types/numeric.h"

// NOTE: Crossing ABI boundaries here, so ensuring that both
// targets agree on the size of the struct!
// So, only use this struct for transfering data. Convert this data into native
// structs for processing. Otherwise, performance will suffer.

// This struct implicitly assumes that there are 4 bytes per pixel, hence a
// U32 buffer
typedef struct __attribute__((packed)) {
    U32 *screen;
    U64 size;
    U32 width;
    U32 height;
    U32 scanline;
} PackedWindow;

typedef struct __attribute__((packed)) {
    U8 *curFree;
    U8 *beg;
    U8 *end;
} PackedArena;

typedef struct __attribute__((packed)) {
    RedBlackNodeMM *tree;
    PackedArena allocator;
    PackedRedBlackNodeMMPtr_a freeList;
} PackedMemoryAllocator;

typedef struct __attribute__((packed)) {
    PackedMemoryAllocator physicalPMA;
    PackedMemoryAllocator virtualPMA;
} PackedKernelMemory;

typedef struct __attribute__((packed)) {
    PackedWindow window;
    PackedKernelMemory memory;
    void *archParams;
} PackedKernelParameters;

void setPackedMemoryAllocator(PackedMemoryAllocator *packedMemoryAllocator,
                              Arena *arena, RedBlackNodeMM *root,
                              RedBlackNodeMMPtr_a *freeList);

#endif
