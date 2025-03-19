#include "x86/memory/virtual.h"

#include "abstraction/memory/manipulation.h"
#include "abstraction/memory/physical/allocation.h"
#include "abstraction/memory/virtual/map.h"
#include "shared/assert.h"
#include "shared/maths/maths.h"
#include "shared/memory/management/definitions.h"
#include "shared/types/types.h"
#include "x86/memory/definitions.h"

VirtualPageTable *rootPageTable;

U8 pageSizeToDepth(PageSize pageSize) {
    switch (pageSize) {
    case X86_4KIB_PAGE: {
        return 4;
    }
    case X86_2MIB_PAGE: {
        return 3;
    }
    default: {
        return 2;
    }
    }
}

static U64 getZeroBasePage() {
    U64 address = getPageForMappingVirtualMemory();
    /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
    memset((void *)address, 0, X86_4KIB_PAGE);
    return address;
}

void mapVirtualRegion(U64 virt, PagedMemory memory, PageSize pageType) {
    mapVirtualRegionWithFlags(virt, memory, pageType, 0);
}

// The caller should take care that the virtual address and physical
// address are correctly aligned. If they are not, not sure what the
// caller wanted to accomplish.
void mapVirtualRegionWithFlags(U64 virt, PagedMemory memory, U64 pageSize,
                               U64 additionalFlags) {
    PageSize pageType = pageSize;
    ASSERT(level4PageTable);
    ASSERT(((virt) >> 48L) == 0 || ((virt) >> 48L) == 0xFFFF);

    ASSERT(!(RING_RANGE_VALUE(virt, pageType)));
    ASSERT(!(RING_RANGE_VALUE(memory.start, pageType)));

    U8 depth = pageSizeToDepth(pageType);
    U64 virtualEnd = virt + pageType * memory.numberOfPages;
    for (U64 physical = memory.start; virt < virtualEnd;
         virt += pageType, physical += pageType) {
        VirtualPageTable *currentTable = rootPageTable;

        U64 pageSize = X86_512GIB_PAGE;
        for (U8 i = 0; i < depth; i++, pageSize /= PageTableFormat.ENTRIES) {
            U64 *address = &(currentTable->pages[RING_RANGE_VALUE(
                (virt / pageSize), PageTableFormat.ENTRIES)]);

            if (i == depth - 1) {
                U64 value = VirtualPageMasks.PAGE_PRESENT |
                            VirtualPageMasks.PAGE_WRITABLE | physical |
                            additionalFlags;
                if (pageType == X86_2MIB_PAGE || pageType == X86_1GIB_PAGE) {
                    value |= VirtualPageMasks.PAGE_EXTENDED_SIZE;
                }
                *address = value;
            } else if (!*address) {
                U64 value = VirtualPageMasks.PAGE_PRESENT |
                            VirtualPageMasks.PAGE_WRITABLE;
                value |= getZeroBasePage();
                *address = value;
            }

            currentTable =
                /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
                (VirtualPageTable *)ALIGN_DOWN_VALUE(*address, X86_4KIB_PAGE);
        }
    }
}
