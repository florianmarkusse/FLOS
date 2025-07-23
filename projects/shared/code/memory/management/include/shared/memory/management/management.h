#ifndef SHARED_MEMORY_MANAGEMENT_MANAGEMENT_H
#define SHARED_MEMORY_MANAGEMENT_MANAGEMENT_H

#include "efi-to-kernel/kernel-parameters.h"
#include "shared/memory/allocator/arena.h"
#include "shared/trees/red-black/memory-manager.h"

typedef struct {
    RedBlackNodeMM_max_a nodes;
    RedBlackNodeMM *tree;
    RedBlackNodeMMPtr_max_a freeList;
} RedBlackMMTreeWithFreeList;

extern RedBlackMMTreeWithFreeList virtualMA;
extern RedBlackMMTreeWithFreeList physicalMA;

void insertRedBlackNodeMMAndAddToFreelist(RedBlackNodeMM **root,
                                          RedBlackNodeMM *newNode,
                                          RedBlackNodeMMPtr_max_a *freeList);
RedBlackNodeMM *getRedBlackNodeMM(RedBlackNodeMMPtr_max_a *freeList,
                                  RedBlackNodeMM_max_a *nodes);

void initMemoryManagers(PackedKernelMemory *kernelMemory);

void *allocVirtualMemory(U64 size, U64 align);
void freeVirtualMemory(Memory memory);

void *allocPhysicalMemory(U64 bytes, U64 align);
void freePhysicalMemory(Memory memory);

#endif
