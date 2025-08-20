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

typedef enum {
    PAGE_FAULT_RESULT_MAPPED,
    PAGE_FAULT_RESULT_STACK_OVERFLOW
} PageFaultResult;

[[nodiscard]] PageFaultResult handlePageFault(U64 faultingAddress);

void addPageMapping(Memory memory, U64_pow2 pageSize);
void removePageMapping(U64 address);

#endif
