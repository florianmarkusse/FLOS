#ifndef SHARED_MEMORY_ALLOCATOR_NODE_H
#define SHARED_MEMORY_ALLOCATOR_NODE_H

#include "shared/types/array-types.h"

// TODO: Ready for code generation. This could be paramterized on the type of
// node to allocate, and then there would be no need for elementSizeBytes
typedef struct {
    void_max_a nodes;
    voidPtr_max_a nodesFreeList;
    U32 elementSizeBytes;
    U32 alignBytes;
} NodeAllocator;

void nodeAllocatorInit(NodeAllocator *nodeAllocator, void_a nodes,
                       void_a nodesFreeList, U32 elementSizeBytes,
                       U32 alignBytes);

[[nodiscard]] void *nodeAllocatorGet(NodeAllocator *nodeAllocator);

void nodeAllocatorFree(NodeAllocator *nodeAllocator, void *nodeFreed);

#endif
