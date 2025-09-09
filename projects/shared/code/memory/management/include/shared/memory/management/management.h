#ifndef SHARED_MEMORY_MANAGEMENT_MANAGEMENT_H
#define SHARED_MEMORY_MANAGEMENT_MANAGEMENT_H

#include "shared/memory/allocator/arena.h"
#include "shared/memory/allocator/node.h"
#include "shared/trees/red-black/memory-manager.h"

typedef struct {
    MMNode *tree;
    NodeAllocator nodeAllocator;
} RedBlackMMTreeWithFreeList;
static_assert(sizeof(RedBlackMMTreeWithFreeList) == 48);

extern RedBlackMMTreeWithFreeList virtualMA;
extern RedBlackMMTreeWithFreeList physicalMA;

void insertMMNodeAndAddToFreelist(RedBlackMMTreeWithFreeList *allocator,
                                  MMNode *newNode);

void *allocVirtualMemory(U64 size, U64_pow2 align);
void freeVirtualMemory(Memory memory);

void *allocPhysicalMemory(U64 bytes, U64_pow2 align);
void freePhysicalMemory(Memory memory);

#endif
