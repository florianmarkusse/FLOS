#ifndef EFI_TO_KERNEL_KERNEL_PARAMETERS_H
#define EFI_TO_KERNEL_KERNEL_PARAMETERS_H

#include "shared/memory/allocator/arena.h"
#include "shared/memory/management/definitions.h"
#include "shared/memory/management/memory-tree.h"
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

typedef PACKED_MAX_LENGTH_ARRAY(U32) PackedU32_max_a;
typedef struct __attribute__((packed)) {
    U8 *base;
    U32 elementSizeBytes;
} PackedNodeLocation;

#define PACKED_TREE_WITH_FREELIST(T)                                           \
    struct __attribute__((packed)) {                                           \
        Packed##T##_max_a nodes;                                               \
        U32 *tree;                                                             \
        PackedU32_max_a freeList;                                              \
        PackedNodeLocation nodeLocation;                                       \
    }

typedef PACKED_MAX_LENGTH_ARRAY(void) Packedvoid_max_a;
typedef PACKED_TREE_WITH_FREELIST(void) PackedTreeWithFreeList;

typedef PACKED_MAX_LENGTH_ARRAY(VMMNode) PackedVMMNode_max_a;
typedef PACKED_TREE_WITH_FREELIST(VMMNode) PackedVMMTreeWithFreeList;

typedef PACKED_MAX_LENGTH_ARRAY(MMNode) PackedMMNode_max_a;
typedef PACKED_TREE_WITH_FREELIST(MMNode) PackedMMTreeWithFreeList;

typedef struct __attribute__((packed)) {
    PackedMMTreeWithFreeList physicalPMA;
    PackedMMTreeWithFreeList virtualPMA;
    PackedVMMTreeWithFreeList virtualMemorySizeMapper;
} PackedKernelMemory;

typedef struct __attribute__((packed)) {
    PackedWindow window;
    PackedKernelMemory memory;
    void *archParams;
} PackedKernelParameters;

void setPackedMemoryAllocator(PackedTreeWithFreeList *packedTreeWithFreeList,
                              TreeWithFreeList *treeWithFreeList);

#endif
