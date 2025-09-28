#ifndef SHARED_MEMORY_MANAGEMENT_MANAGEMENT_H
#define SHARED_MEMORY_MANAGEMENT_MANAGEMENT_H

#include "shared/memory/allocator/arena.h"
#include "shared/memory/allocator/buddy.h"
#include "shared/memory/allocator/node.h"
#include "shared/trees/red-black/memory-manager.h"

typedef struct {
    MMNode *tree;
    NodeAllocator nodeAllocator;
} RedBlackMMTreeWithFreeList;
static_assert(sizeof(RedBlackMMTreeWithFreeList) == 48);

typedef struct {
    NodeAllocator nodeAllocator;
    Buddy buddy;
} BuddyWithNodeAllocator;

extern BuddyWithNodeAllocator buddyPhysical;
extern BuddyWithNodeAllocator buddyVirtual;

void *allocVirtualMemory(U64_pow2 blockSize);
void freeVirtualMemory(Memory memory);

void *allocPhysicalMemory(U64_pow2 blockSize);
void freePhysicalMemory(Memory memory);

#endif
