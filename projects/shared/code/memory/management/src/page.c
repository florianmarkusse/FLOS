#include "shared/memory/management/page.h"
#include "abstraction/interrupts.h"
#include "abstraction/log.h"
#include "abstraction/memory/virtual/converter.h"
#include "abstraction/memory/virtual/map.h"
#include "abstraction/thread.h"
#include "shared/log.h"
#include "shared/maths.h"
#include "shared/memory/converter.h"
#include "shared/memory/management/management.h"
#include "shared/memory/sizes.h"

VMMTreeWithFreeList memoryMapperSizes = {0};

static U64_pow2 pageSizeFromVMM(U64 faultingAddress) {
    VMMNode *result = VMMNodeFindGreatestBelowOrEqual(&memoryMapperSizes.tree,
                                                      faultingAddress);
    if (result && result->basic.value + result->bytes > faultingAddress) {
        return result->mappingSize;
    }

    return pageSizeSmallest();
}

void pageMappingRemove(U64 address) {
    VMMNode *deleted = VMMNodeDelete(&memoryMapperSizes.tree, address);
    nodeAllocatorFree(&memoryMapperSizes.nodeAllocator, deleted);
}

void pageMappingAdd(Memory memory, U64_pow2 pageSize) {
    VMMNode *newNode = nodeAllocatorGet(&memoryMapperSizes.nodeAllocator);
    if (!newNode) {
        interruptVirtualMemoryMapper();
    }
    newNode->basic.value = memory.start;
    newNode->bytes = memory.bytes;
    newNode->mappingSize = pageSize;

    VMMNodeInsert(&memoryMapperSizes.tree, newNode);
}

PageFaultResult pageFaultHandle(U64 faultingAddress) {
    U64_pow2 pageSizeForFault = pageSizeFromVMM(faultingAddress);
    if (pageSizeForFault == GUARD_PAGE_SIZE) {
        return PAGE_FAULT_RESULT_STACK_OVERFLOW;
    }

    U64 startingMap = alignDown(faultingAddress, pageSizeForFault);
    U64_pow2 pageSizeToUse = pageSizeFitting(pageSizeForFault);

    // NOTE: when starting to use SMP, I should first check if this memory
    // is now mapped before doing this.
    // In the context of mapping and unmapping. It may be interesting to
    // mark which cores have accessed which memory so we can limit the
    // flushPage calls to all cores.

    U32_pow2 mapsToDo = (U32)dividePowerOf2(pageSizeForFault, pageSizeToUse);
    for (U32 i = 0; i < mapsToDo; i++) {
        U8 *address = physicalMemoryAlloc(pageSizeToUse);
        pageMap(startingMap + (i * pageSizeToUse), (U64)address, pageSizeToUse);
    }

    return PAGE_FAULT_RESULT_MAPPED;
}
