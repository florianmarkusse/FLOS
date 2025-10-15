#ifndef SHARED_MEMORY_MANAGEMENT_PAGE_H
#define SHARED_MEMORY_MANAGEMENT_PAGE_H

#include "shared/memory/allocator/node.h"
#include "shared/trees/red-black/virtual-mapping-manager.h"
#include "shared/types/numeric.h"

typedef struct {
    VMMNode *tree;
    NodeAllocator nodeAllocator;
} VMMTreeWithFreeList;

extern VMMTreeWithFreeList memoryMapperSizes;

typedef enum {
    PAGE_FAULT_RESULT_MAPPED,
    PAGE_FAULT_RESULT_STACK_OVERFLOW
} PageFaultResult;

[[nodiscard]] PageFaultResult handlePageFault(U64 faultingAddress);

static constexpr U64_pow2 GUARD_PAGE_SIZE = 0;

void addPageMapping(Memory memory, U64_pow2 pageSize);
void removePageMapping(U64 address);

#endif
