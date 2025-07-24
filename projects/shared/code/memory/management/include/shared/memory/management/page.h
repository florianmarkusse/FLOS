#ifndef SHARED_MEMORY_MANAGEMENT_PAGE_H
#define SHARED_MEMORY_MANAGEMENT_PAGE_H

#include "shared/trees/red-black/virtual-mapping-manager.h"
#include "shared/types/numeric.h"

typedef struct {
    RedBlackVMM_max_a nodes;
    VMMNode *tree;
    RedBlackVMMPtr_max_a freeList;
} VMMTreeWithFreeList;

extern VMMTreeWithFreeList virtualMemorySizeMapper;

void handlePageFault(U64 faultingAddress);

void addPageMapping(Memory memory, U64 pageSize);
void removePageMapping(U64 address);

#endif
