#ifndef EFI_TO_KERNEL_KERNEL_PARAMETERS_H
#define EFI_TO_KERNEL_KERNEL_PARAMETERS_H

#include "shared/memory/allocator/arena.h"
#include "shared/memory/allocator/node.h"
#include "shared/memory/management/definitions.h"
#include "shared/memory/management/management.h"
#include "shared/memory/management/page.h"
#include "shared/trees/red-black/memory-manager.h"
#include "shared/trees/red-black/virtual-mapping-manager.h"
#include "shared/types/array.h"
#include "shared/types/numeric.h"

// NOTE: Crossing ABI boundaries here, so ensuring that both
// targets agree on the size of the struct!
// So, only use this struct for transfering data. Convert this data into native
// structs for processing. Otherwise, performance will suffer.

// This struct implicitly assumes that there are 4 bytes per pixel, hence a
// U32 buffer
typedef struct {
    U32 *pixels;
    U64 size;
    U32 width;
    U32 height;
    U32 scanline;
} Window;
static_assert(sizeof(Window) == 32);

typedef struct {
    U8 *curFree;
    U8 *beg;
    U8 *end;
} PackedArena;
static_assert(sizeof(PackedArena) == 24);

typedef struct {
    RedBlackMMTreeWithFreeList physicalPMA;
    RedBlackMMTreeWithFreeList virtualPMA;
    VMMTreeWithFreeList virtualMemorySizeMapper;
} KernelMemory;

typedef struct {
    Window window;
    KernelMemory memory;
    void *archParams;
} KernelParameters;

#endif
