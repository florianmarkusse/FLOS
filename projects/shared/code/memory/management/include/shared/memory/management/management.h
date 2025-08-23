#ifndef SHARED_MEMORY_MANAGEMENT_MANAGEMENT_H
#define SHARED_MEMORY_MANAGEMENT_MANAGEMENT_H

#include "efi-to-kernel/kernel-parameters.h"
#include "shared/memory/allocator/arena.h"
#include "shared/trees/red-black/memory-manager.h"

typedef struct {
    MMNode_max_a nodes;
    MMNode *tree;
    MMNodePtr_max_a freeList;
} RedBlackMMTreeWithFreeList;

extern RedBlackMMTreeWithFreeList virtualMA;
extern RedBlackMMTreeWithFreeList physicalMA;

void insertMMNodeAndAddToFreelist(MMNode **root, MMNode *newNode,
                                  MMNodePtr_max_a *freeList);

void initMemoryManagers(PackedKernelMemory *kernelMemory);

void *allocVirtualMemory(U64 size, U64_pow2 align);
void freeVirtualMemory(Memory memory);

void *allocPhysicalMemory(U64 bytes, U64_pow2 align);
void freePhysicalMemory(Memory memory);

#endif
