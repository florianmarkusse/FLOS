#ifndef EFI_TO_KERNEL_KERNEL_PARAMETERS_H
#define EFI_TO_KERNEL_KERNEL_PARAMETERS_H

#include "shared/memory/management/definitions.h"
#include "shared/trees/red-black.h"
#include "shared/types/types.h"

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
    RedBlackNode *tree;
    Arena allocator;
} PhysicalMemory;

typedef struct {
    U64 availableLowerHalfAddress;
    U64 availableHigherHalfAddress;
} VirtualMemory;

typedef struct {
    PhysicalMemory physical;
    VirtualMemory virt;
} KernelMemory;

typedef struct {
    Window window;
    KernelMemory memory;
} KernelParameters;

#endif
