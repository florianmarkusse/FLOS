#include "x86/memory/virtual.h"

#include "abstraction/memory/manipulation.h"
#include "abstraction/memory/physical/allocation.h"
#include "abstraction/memory/virtual/map.h"
#include "shared/assert.h"
#include "shared/maths/maths.h"
#include "shared/memory/converter.h"
#include "shared/memory/management/definitions.h"
#include "shared/types/types.h"
#include "x86/memory/definitions.h"
#include "x86/memory/flags.h"

VirtualPageTable *rootPageTable;

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
    U64 address = getPageForMappingVirtualMemory();
    /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
    memset((void *)address, 0, X86_4KIB_PAGE);
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
