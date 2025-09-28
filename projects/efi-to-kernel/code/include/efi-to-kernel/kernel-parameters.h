#ifndef EFI_TO_KERNEL_KERNEL_PARAMETERS_H
#define EFI_TO_KERNEL_KERNEL_PARAMETERS_H

#include "abstraction/efi-to-kernel.h"
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
    NodeAllocator nodeAllocator;
    BuddyData data;
} BuddyDataWithNodeAllocator;

typedef struct {
    BuddyDataWithNodeAllocator buddyPhysical;
    BuddyDataWithNodeAllocator buddyVirtual;
    VMMTreeWithFreeList memoryMapperSizes;
} KernelMemory;

struct KernelParameters {
    Window window;
    KernelMemory memory;
    // TODO: On new computer with newer UEFI, we can remove permanent free
    // because we can use custom memory types
    Memory permanentLeftoverFree; // NOTE: Immediately freeable upon entering
                                  // kernelmain
    Memory self;                  // NOTE: Freeable once init code is complete
    alignas(ARCH_PARAMS_ALIGNMENT) U8 archParams[];
};

#endif
