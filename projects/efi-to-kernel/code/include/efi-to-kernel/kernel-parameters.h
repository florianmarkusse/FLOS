#ifndef EFI_TO_KERNEL_KERNEL_PARAMETERS_H
#define EFI_TO_KERNEL_KERNEL_PARAMETERS_H

#include "shared/memory/management/definitions.h"
#include "shared/trees/red-black.h"
#include "shared/types/types.h"

typedef struct {
    U64 ptr;
    U64 size;
    U32 columns;
    U32 rows;
    U32 scanline;
} FrameBuffer;

typedef struct {
    RedBlackNode *tree;
    Arena allocator;
} KernelMemory;

typedef struct {
    FrameBuffer fb;
    KernelMemory kernelMemory;
} KernelParameters;

#endif
