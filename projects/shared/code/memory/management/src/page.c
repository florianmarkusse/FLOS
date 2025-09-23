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

VMMTreeWithFreeList virtualMemorySizeMapper = {0};
bool canLog = false;

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

PageFaultResult handlePageFault(U64 faultingAddress, U8 *temp, U8 *now) {
    if (canLog) {
        // KFLUSH_AFTER {
        //     KLOG(STRING("The faulting address is: "));
        //     KLOG((void *)faultingAddress, .flags = NEWLINE);
        // }
    }
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
        U8 *address =
            allocPhysicalMemoryTest(pageSizeToUse, pageSizeToUse, temp, now);

        // if (canLog && temp && now) {
        //     KFLUSH_AFTER {
        //         for (U32 i = 0; i < 832; i++) {
        //             if (temp[i] != now[i]) {
        //                 INFO(STRING("Difference at i = "));
        //                 INFO(i);
        //                 INFO(STRING(" orig: "));
        //                 INFO(temp[i]);
        //                 INFO(STRING(" now: "));
        //                 INFO(now[i], .flags = NEWLINE);
        //             }
        //         }
        //     }
        // }

        if (canLog) {
            INFO(STRING("mapping "));
            INFO((void *)(startingMap + (i * pageSizeToUse)));
            INFO(STRING(" to "));
            INFO(address);
        }
        mapPage(startingMap + (i * pageSizeToUse), (U64)address, pageSizeToUse);
    }

    if (canLog && temp && now) {
        KFLUSH_AFTER {
            for (U32 i = 0; i < 832; i++) {
                if (temp[i] != now[i]) {
                    INFO(STRING("Difference at i = "));
                    INFO(i);
                    INFO(STRING(" orig: "));
                    INFO(temp[i]);
                    INFO(STRING(" now: "));
                    INFO(now[i], .flags = NEWLINE);
                }
            }
        }
    }

    return PAGE_FAULT_RESULT_MAPPED;
}
