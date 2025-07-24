#include "shared/memory/management/page.h"
#include "abstraction/interrupts.h"
#include "abstraction/memory/virtual/converter.h"
#include "abstraction/memory/virtual/map.h"
#include "shared/maths.h"
#include "shared/memory/converter.h"
#include "shared/memory/management/management.h"
#include "shared/memory/sizes.h"

VMMTreeWithFreeList virtualMemorySizeMapper = {0};

static U64 pageSizeFromVMM(U64 faultingAddress) {
    VMMNode *result = findGreatestBelowOrEqualVMMNode(
        &virtualMemorySizeMapper.tree, faultingAddress);
    if (result && result->basic.value + result->bytes > faultingAddress) {
        return result->mappingSize;
    }

    return SMALLEST_VIRTUAL_PAGE;
}

// TODO: Ready for code generation
static VMMNode *getVMM(RedBlackVMMPtr_max_a *freeList,
                       RedBlackVMM_max_a *nodes) {
    if (freeList->len > 0) {
        VMMNode *result = freeList->buf[freeList->len - 1];
        freeList->len--;
        return result;
    }

    if (nodes->len < nodes->cap) {
        VMMNode *result = &nodes->buf[nodes->len];
        nodes->len++;
        return result;
    }

    return nullptr;
}

void removePageMapping(U64 address) {
    VMMNode *deleted = deleteVMMNode(&virtualMemorySizeMapper.tree, address);
    virtualMemorySizeMapper.freeList.buf[virtualMemorySizeMapper.freeList.len] =
        deleted;
    virtualMemorySizeMapper.freeList.len++;
}

void addPageMapping(Memory memory, U64 pageSize) {
    VMMNode *newNode = getVMM(&virtualMemorySizeMapper.freeList,
                              &virtualMemorySizeMapper.nodes);
    if (!newNode) {
        interruptUnexpectedError();
    }
    newNode->basic.value = memory.start;
    newNode->bytes = memory.bytes;
    newNode->mappingSize = pageSize;

    insertVMMNode(&virtualMemorySizeMapper.tree, newNode);
}

void handlePageFault(U64 faultingAddress) {
    U64 pageSizeForFault = pageSizeFromVMM(faultingAddress);

    U64 startingMap = ALIGN_DOWN_VALUE(faultingAddress, pageSizeForFault);
    U64 pageSizeToUse = pageSizeFitting(pageSizeForFault);

    // NOTE: when starting to use SMP, I should first check if this memory
    // is now mapped before doing this.
    // In the context of mapping and unmapping. It may be interesting to
    // mark which cores have accessed which memory so we can limit the
    // flushPage calls to all cores.

    U64 mapsToDo = divideByPowerOf2(pageSizeForFault, pageSizeToUse);
    for (U64 i = 0; i < mapsToDo; i++) {
        U8 *address = allocPhysicalMemory(pageSizeToUse, pageSizeToUse);
        mapPage(startingMap + (i * pageSizeToUse), (U64)address, pageSizeToUse);
    }
}
