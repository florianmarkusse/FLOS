#ifndef SHARED_MEMORY_MANAGEMENT_PAGE_H
#define SHARED_MEMORY_MANAGEMENT_PAGE_H

#include "shared/trees/red-black/virtual-mapping-manager.h"
#include "shared/types/numeric.h"

typedef struct {
    RedBlackVMM_max_a nodes;
    RedBlackVMM *tree;
    RedBlackVMMPtr_max_a freeList;
} VirtualMemorySizeMapper;

extern VirtualMemorySizeMapper virtualMemorySizeMapper;

void handlePageFault(U64 faultingAddress);

RedBlackVMM *getRedBlackVMM(RedBlackVMMPtr_max_a *freeList,
                            RedBlackVMM_max_a *nodes);
void addPageMapping(Memory memory, U64 pageSize);

#endif
