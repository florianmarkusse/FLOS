#ifndef EFI_TO_KERNEL_KERNEL_PARAMETERS_H
#define EFI_TO_KERNEL_KERNEL_PARAMETERS_H

#include "shared/memory/management/definitions.h"
#include "shared/types/types.h"
typedef struct {
    U64 ptr;
    U64 size;
    U32 columns;
    U32 rows;
    U32 scanline;
} FrameBuffer;

typedef struct {
    PagedMemory_a memory;
    U64 UEFIPages;
} KernelMemory;

typedef struct {
    FrameBuffer fb;
    KernelMemory kernelMemory;
} KernelParameters;

#endif
