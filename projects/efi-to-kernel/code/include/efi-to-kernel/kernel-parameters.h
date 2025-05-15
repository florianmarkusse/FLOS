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
typedef struct {
    U32 *screen;
    U64 size;
    U32 width;
    U32 height;
    U32 scanline;
} __attribute__((packed)) PackedWindow;

typedef struct {
    U8 *curFree;
    U8 *beg;
    U8 *end;
} __attribute__((packed)) PackedArena;

typedef struct {
    RedBlackNodeMM *tree;
    PackedArena allocator;
} __attribute__((packed)) PackedMemoryTree;

typedef struct {
    PackedMemoryTree physical;
    PackedMemoryTree virt;
} __attribute__((packed)) PackedKernelMemory;

typedef struct {
    U64 tscFrequencyPerMicroSecond;
    U64 rootVirtualMetaDataAddress;
    U64 rootReferenceCount;
} __attribute__((packed)) PackedArchitectureInit;

typedef struct {
    PackedWindow window;
    PackedKernelMemory memory;
    PackedArchitectureInit archInit; // This can be different per core?
} __attribute__((packed)) PackedKernelParameters;

#endif
