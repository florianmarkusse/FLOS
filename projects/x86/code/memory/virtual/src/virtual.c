#include "x86/memory/virtual.h"

#include "abstraction/memory/manipulation.h"
#include "abstraction/memory/physical.h"
#include "shared/assert.h"
#include "shared/maths/maths.h"
#include "shared/memory/converter.h"
#include "shared/memory/management/definitions.h"
#include "shared/types/numeric.h"
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
VirtualMetaData *virtualMetaData;

static U64 getZeroBasePage() {
    U64 address = getPageForMappingVirtualMemory(
        VIRTUAL_MEMORY_MAPPING_SIZE, VIRTUAL_MEMORY_MAPPER_ALIGNMENT);
    /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
    memset((void *)address, 0, VIRTUAL_MEMORY_MAPPING_SIZE);
    return address;
}

static U16 calculateTableIndex(U64 virt, U64 pageSize) {
    return RING_RANGE_VALUE((virt / pageSize), PageTableFormat.ENTRIES);
}

// The caller should take care that the physical address is correctly aligned.
// If it is not, not sure what the caller wanted to accomplish.
void mapPageWithFlags(U64 virt, U64 physical, U64 mappingSize, U64 flags) {
    ASSERT(rootPageTable);
    ASSERT(((virt) >> 48L) == 0 || ((virt) >> 48L) == 0xFFFF);
    ASSERT(!(RING_RANGE_VALUE(physical, mappingSize)));

    VirtualPageTable *table = rootPageTable;
    U64 *entryAddress;
    for (U64 pageSize = X86_512GIB_PAGE; pageSize >= mappingSize;
         pageSize /= PageTableFormat.ENTRIES) {
        U16 index = calculateTableIndex(virt, pageSize);
        entryAddress = &(table->pages[index]);

        if (pageSize == mappingSize) {
            U64 value = physical | flags;
            if (mappingSize == X86_2MIB_PAGE || mappingSize == X86_1GIB_PAGE) {
                value |= VirtualPageMasks.PAGE_EXTENDED_SIZE;
            }
            *entryAddress = value;
            return;
        }

        if (!(*entryAddress)) {
            U64 value = KERNEL_STANDARD_PAGE_FLAGS;
            value |= getZeroBasePage();
            *entryAddress = value;
        }

        table =
            /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
            (VirtualPageTable *)ALIGN_DOWN_VALUE(*entryAddress,
                                                 SMALLEST_VIRTUAL_PAGE);
    }
}

void mapPage(U64 virt, U64 physical, U64 mappingSize) {
    return mapPageWithFlags(virt, physical, mappingSize,
                            KERNEL_STANDARD_PAGE_FLAGS);
}

U64 getPhysicalAddressFrame(U64 virtualPage) {
    return virtualPage & VirtualPageMasks.FRAME_OR_NEXT_PAGE_TABLE;
}

Memory unmapPage(U64 virt) {
    ASSERT(rootPageTable);
    ASSERT(((virt) >> 48L) == 0 || ((virt) >> 48L) == 0xFFFF);

    U64 entry;
    VirtualPageTable *table = rootPageTable;
    for (U64 pageSize = X86_512GIB_PAGE; pageSize >= SMALLEST_VIRTUAL_PAGE;
         pageSize /= PageTableFormat.ENTRIES) {
        U16 index = calculateTableIndex(virt, pageSize);
        entry = table->pages[index];

        if (!entry || pageSize == SMALLEST_VIRTUAL_PAGE) {
            return (Memory){.start = getPhysicalAddressFrame(entry),
                            .bytes = pageSize};
        }

        // NOTE: No possibility of conflicting with bad PATs here because we are
        // not using them.
        if ((entry & VirtualPageMasks.PAGE_EXTENDED_SIZE)) {
            return (Memory){.start = getPhysicalAddressFrame(entry),
                            .bytes = pageSize};
        }

        table =
            /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
            (VirtualPageTable *)ALIGN_DOWN_VALUE(entry, SMALLEST_VIRTUAL_PAGE);
    }

    __builtin_unreachable();
}
