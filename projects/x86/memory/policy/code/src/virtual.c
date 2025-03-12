#include "x86/memory/policy/virtual.h"
#include "abstraction/memory/management/policy.h"

#include "efi-to-kernel/memory/definitions.h"
#include "shared/assert.h"
#include "shared/maths/maths.h"
#include "shared/types/types.h"
#include "x86/configuration/cpu.h"
#include "x86/memory/pat.h"
#include "x86/memory/virtual.h"

VirtualRegion higherHalfRegion = {.start = HIGHER_HALF_START,
                                  .end = KERNEL_SPACE_START};
// Start is set in the init function.
VirtualRegion lowerHalfRegion = {.start = 0, .end = LOWER_HALF_END};

U64 getVirtualMemory(U64 size, PageSize alignValue) {
    ASSERT(size <= X86_512GIB_PAGE);
    U64 alignedUpValue = ALIGN_UP_VALUE(higherHalfRegion.start, alignValue);

    ASSERT(higherHalfRegion.start <= alignedUpValue);
    ASSERT(alignedUpValue <= higherHalfRegion.end);
    ASSERT(higherHalfRegion.end - (alignedUpValue + size) <
           higherHalfRegion.start);

    higherHalfRegion.start = alignedUpValue;
    U64 result = higherHalfRegion.start;

    higherHalfRegion.start += size;
    return result;
}

U64 getPhysicalAddressFrame(U64 virtualPage) {
    return virtualPage & VirtualPageMasks.FRAME_OR_NEXT_PAGE_TABLE;
}

void initVirtualMemoryManager(KernelMemory kernelMemory) {
    U64 currentHighestAddress = 0;
    for (U64 i = 0; i < kernelMemory.memory.len; i++) {
        PagedMemory paged = kernelMemory.memory.buf[i];
        U64 highestAddress = paged.start + paged.numberOfPages * X86_4KIB_PAGE;

        if (highestAddress > currentHighestAddress) {
            currentHighestAddress = highestAddress;
        }
    }
    lowerHalfRegion.start = currentHighestAddress;

    /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
    level4PageTable = (VirtualPageTable *)CR3();
}

static bool isExtendedPageLevel(U8 level) {
    return level == pageSizeToDepth(X86_2MIB_PAGE) ||
           level == pageSizeToDepth(X86_1GIB_PAGE);
}

MappedPage getMappedPage(U64 virtual) {
    MappedPage result;
    result.pageSize = X86_256TIB_PAGE;

    U64 *address;
    U64 pageSize = X86_512GIB_PAGE;
    VirtualPageTable *currentTable = level4PageTable;
    U8 totalDepth = pageSizeToDepth(availablePageSizes[0]);
    for (U8 level = 0; level < totalDepth;
         level++, pageSize /= PageTableFormat.ENTRIES) {
        address = &(currentTable->pages[RING_RANGE_VALUE(
            (virtual / pageSize), PageTableFormat.ENTRIES)]);
        result.pageSize /= PageTableFormat.ENTRIES;

        if (isExtendedPageLevel(level) &&
            ((*address) & VirtualPageMasks.PAGE_EXTENDED_SIZE)) {
            result.entry = *(VirtualEntry *)address;
            return result;
        }

        currentTable =
            /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
            (VirtualPageTable *)ALIGN_DOWN_VALUE(*address, X86_4KIB_PAGE);
    }

    result.entry = *(VirtualEntry *)address;
    return result;
}
