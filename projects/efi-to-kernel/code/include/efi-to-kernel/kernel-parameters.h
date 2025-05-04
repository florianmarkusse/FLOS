#ifndef EFI_TO_KERNEL_KERNEL_PARAMETERS_H
#define EFI_TO_KERNEL_KERNEL_PARAMETERS_H

#include "shared/memory/allocator/arena.h"
#include "shared/memory/management/definitions.h"
#include "shared/trees/red-black/memory-manager.h"
#include "shared/types/numeric.h"

#pragma pack(push, 1)

// This struct implicitly assumes that there are 4 bytes per pixel, hence a
// U32 buffer
typedef struct {
    U32 *screen;
    U32 *backingBuffer;
    U64 size;
    U32 width;
    U32 height;
    U32 scanline;
} Window;

typedef struct {
    RedBlackNodeMM *tree;
    Arena allocator;
} MemoryTree;

typedef struct {
    MemoryTree physical;
    MemoryTree virt;
} KernelMemory;

typedef struct {
    Window window;
    KernelMemory memory;
} __attribute__((packed))
KernelParameters; // NOTE: Crossing ABI boundaries here, so ensuring that both
                  // targets agree on the size of the struct!

#pragma pack(pop)
#endif
