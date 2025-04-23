#include "x86/memory/virtual.h"

#include "abstraction/memory/manipulation.h"
#include "abstraction/memory/physical.h"
#include "abstraction/memory/virtual/map.h"
#include "shared/assert.h"
#include "shared/maths/maths.h"
#include "shared/memory/converter.h"
#include "shared/memory/management/definitions.h"
#include "shared/memory/physical.h"
#include "shared/types/types.h"
#include "x86/configuration/cpu.h"
#include "x86/memory/definitions.h"
#include "x86/memory/flags.h"

// static string patEncodingToString[PAT_ENCODING_COUNT] = {
//     STRING("Uncachable (UC)"),        STRING("Write Combining (WC)"),
//     STRING("Reserved 1, don't use!"), STRING("Reserved 2, don't use!"),
//     STRING("Write Through (WT)"),     STRING("Write Protected (WP)"),
//     STRING("Write Back (WB)"),        STRING("Uncached (UC-)"),
// };

VirtualPageTable *rootPageTable;

void setRootPageTable() { rootPageTable = (VirtualPageTable *)CR3(); }

U8 pageSizeToDepth(PageSize pageSize) {
    switch (pageSize) {
    case X86_4KIB_PAGE: {
        return 4;
    }
    case X86_2MIB_PAGE: {
        return 3;
    }
    case X86_1GIB_PAGE: {
        return 2;
    }
    }
}

static U64 getZeroBasePage() {
    U64 address = getPageForMappingVirtualMemory(VIRTUAL_MEMORY_MAPPING_SIZE);
    /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
    memset((void *)address, 0, VIRTUAL_MEMORY_MAPPING_SIZE);
    return address;
}

// The caller should take care that the virtual address and physical
// address are correctly aligned. If they are not, not sure what the
// caller wanted to accomplish.
void mapPageWithFlags(U64 virt, U64 physical, U64 mappingSize, U64 flags) {
    ASSERT(rootPageTable);
    ASSERT(((virt) >> 48L) == 0 || ((virt) >> 48L) == 0xFFFF);
    ASSERT(!(RING_RANGE_VALUE(virt, mappingSize)));
    ASSERT(!(RING_RANGE_VALUE(physical, mappingSize)));

    U8 depth = pageSizeToDepth(mappingSize);
    VirtualPageTable *currentTable = rootPageTable;

    U64 pageSize = X86_512GIB_PAGE;
    for (U8 i = 0; i < depth; i++, pageSize /= PageTableFormat.ENTRIES) {
        U64 *address = &(currentTable->pages[RING_RANGE_VALUE(
            (virt / pageSize), PageTableFormat.ENTRIES)]);

        if (i == depth - 1) {
            U64 value = physical | flags;
            if (pageSize == X86_2MIB_PAGE || pageSize == X86_1GIB_PAGE) {
                value |= VirtualPageMasks.PAGE_EXTENDED_SIZE;
            }
            *address = value;
        } else if (!*address) {
            U64 value = KERNEL_STANDARD_PAGE_FLAGS;
            value |= getZeroBasePage();
            *address = value;
        }

        currentTable =
            /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
            (VirtualPageTable *)ALIGN_DOWN_VALUE(*address, X86_4KIB_PAGE);
    }
}

void mapPage(U64 virt, U64 physical, U64 mappingSize) {
    return mapPageWithFlags(virt, physical, mappingSize,
                            KERNEL_STANDARD_PAGE_FLAGS);
}

U64 getPhysicalAddressFrame(U64 virtualPage) {
    return virtualPage & VirtualPageMasks.FRAME_OR_NEXT_PAGE_TABLE;
}

static bool isExtendedPageLevel(U8 level) {
    return level == pageSizeToDepth(X86_2MIB_PAGE) ||
           level == pageSizeToDepth(X86_1GIB_PAGE);
}

MappedPage getMappedPage(U64 virt) {
    MappedPage result;
    result.pageSize = X86_256TIB_PAGE;

    U64 *address;
    U64 pageSize = X86_512GIB_PAGE;
    VirtualPageTable *currentTable = rootPageTable;
    U8 totalDepth = pageSizeToDepth(availablePageSizes[0]);
    for (U8 level = 0; level < totalDepth;
         level++, pageSize /= PageTableFormat.ENTRIES) {
        address = &(currentTable->pages[RING_RANGE_VALUE(
            (virt / pageSize), PageTableFormat.ENTRIES)]);
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
