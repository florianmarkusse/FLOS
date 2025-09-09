#ifndef EFI_TO_KERNEL_KERNEL_PARAMETERS_H
#define EFI_TO_KERNEL_KERNEL_PARAMETERS_H

#include "shared/memory/allocator/arena.h"
#include "shared/memory/allocator/node.h"
#include "shared/memory/management/definitions.h"
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
    U32 *screen;
    U64 size;
    U32 width;
    U32 height;
    U32 scanline;
} PackedWindow;
static_assert(sizeof(PackedWindow) == 32);

typedef struct {
    U8 *curFree;
    U8 *beg;
    U8 *end;
} PackedArena;
static_assert(sizeof(PackedArena) == 24);

typedef PACKED_MAX_LENGTH_ARRAY(void) PackedVoid_max_a;
typedef PACKED_MAX_LENGTH_ARRAY(void *) PackedVoidPtr_max_a;
typedef struct __attribute__((packed)) {
    PackedVoid_max_a nodes;
    PackedVoidPtr_max_a nodesFreeList;
    U32 elementSizeBytes;
} PackedNodeAllocator;

typedef struct __attribute__((packed)) {
    VMMNode *tree;
    PackedNodeAllocator nodeAllocator;
} PackedVMMTreeWithFreeList;

typedef struct __attribute__((packed)) {
    U64 start;
    U64 bytes;
} PackedMemory;

typedef struct __attribute__((packed)) PackedMMNode PackedMMNode;
struct PackedMMNode {
    PackedMMNode *
        children[RB_TREE_CHILD_COUNT]; // NOTE: Keep this as the first elements.
                                       // This is used in the insert so that
                                       // children->[0] and a RedBlackNode* are
                                       // the same location for doing inserts.
    RedBlackColor color;               // NOTE: Keep this as the second element
    PackedMemory memory;
    U64 mostBytesInSubtree;
};

typedef struct __attribute__((packed)) {
    MMNode *tree;
    PackedNodeAllocator nodeAllocator;
} PackedMMTreeWithFreeList;

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

void setPackedMemoryAllocator(PackedNodeAllocator *packedNodeAllocator,
                              void **packedTree, NodeAllocator *nodeAllocator,
                              void *tree);

#endif
