#include "shared/memory/management/page.h"
#include "abstraction/interrupts.h"
#include "abstraction/log.h"
#include "abstraction/memory/virtual/converter.h"
#include "abstraction/memory/virtual/map.h"
#include "abstraction/thread.h"
#include "shared/maths.h"
#include "shared/memory/converter.h"
#include "shared/memory/management/management.h"
#include "shared/memory/sizes.h"

VMMTreeWithFreeList virtualMemorySizeMapper = {0};

static U64_pow2 pageSizeFromVMM(U64 faultingAddress) {
    VMMNode *result = findGreatestBelowOrEqualVMMNode(
        &virtualMemorySizeMapper.tree, faultingAddress);
    if (result && result->basic.value + result->bytes > faultingAddress) {
        return result->mappingSize;
    }

    return pageSizesSmallest();
}

void removePageMapping(U64 address) {
    VMMNode *deleted = deleteVMMNode(&virtualMemorySizeMapper.tree, address);
    nodeAllocatorFree(&virtualMemorySizeMapper.nodeAllocator, deleted);
}

void addPageMapping(Memory memory, U64_pow2 pageSize) {
    VMMNode *newNode = nodeAllocatorGet(&virtualMemorySizeMapper.nodeAllocator);
    if (!newNode) {
        interruptUnexpectedError();
    }
    newNode->basic.value = memory.start;
    newNode->bytes = memory.bytes;
    newNode->mappingSize = pageSize;

    insertVMMNode(&virtualMemorySizeMapper.tree, newNode);
}

PageFaultResult handlePageFault(U64 faultingAddress) {
    U64_pow2 pageSizeForFault = pageSizeFromVMM(faultingAddress);

    if (!pageSizeForFault) {
        return PAGE_FAULT_RESULT_STACK_OVERFLOW;
    }

    U64 startingMap = alignDown(faultingAddress, pageSizeForFault);
    U64_pow2 pageSizeToUse = pageSizeFitting(pageSizeForFault);

    // NOTE: when starting to use SMP, I should first check if this memory
    // is now mapped before doing this.
    // In the context of mapping and unmapping. It may be interesting to
    // mark which cores have accessed which memory so we can limit the
    // flushPage calls to all cores.

    U32_pow2 mapsToDo = (U32)divideByPowerOf2(pageSizeForFault, pageSizeToUse);
    for (U32 i = 0; i < mapsToDo; i++) {
        U8 *address = allocPhysicalMemory(pageSizeToUse, pageSizeToUse);
        mapPage(startingMap + (i * pageSizeToUse), (U64)address, pageSizeToUse);
    }

    return PAGE_FAULT_RESULT_MAPPED;
}
