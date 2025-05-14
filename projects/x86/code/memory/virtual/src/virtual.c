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
VirtualMetaData *rootVirtualMetaData;

static U64 getZeroedMemory(U64 bytes, U64 align) {
    U64 address = getBytesForMemoryMapping(bytes, align);
    /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
    memset((void *)address, 0, bytes);
    return address;
}

static U64 getZeroedMetaData() {
    return getZeroedMemory(sizeof(VirtualMetaData), alignof(VirtualMetaData));
}

static U64 getZeroedMetaDataTable() {
    return getZeroedMemory(PageTableFormat.ENTRIES *
                               sizeof(rootVirtualMetaData),
                           alignof(typeof(VirtualMetaData)));
}

static U64 getZeroedPageTable() {
    return getZeroedMemory(VIRTUAL_MEMORY_MAPPING_SIZE,
                           VIRTUAL_MEMORY_MAPPER_ALIGNMENT);
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

    VirtualMetaData *metaData[MAX_PAGING_LEVELS];
    metaData[0] = rootVirtualMetaData;
    U64 *metaEntryAddress;

    VirtualPageTable *pageTables[MAX_PAGING_LEVELS];
    pageTables[0] = rootPageTable;
    U64 *tableEntryAddress;

    U8 len = 1;

    for (U64 pageSize = X86_512GIB_PAGE; pageSize >= mappingSize;
         pageSize /= PageTableFormat.ENTRIES) {
        U16 index = calculateTableIndex(virt, pageSize);
        tableEntryAddress = &(pageTables[len - 1]->pages[index]);
        metaEntryAddress = (U64 *)&(metaData[len - 1]->pages[index]);

        if (pageSize == mappingSize) {
            U64 value = physical | flags;
            if (mappingSize == X86_2MIB_PAGE || mappingSize == X86_1GIB_PAGE) {
                value |= VirtualPageMasks.PAGE_EXTENDED_SIZE;
            }
            *tableEntryAddress = value;
            metaData[len - 1]->count++;
            return;
        }

        if (!(*tableEntryAddress)) {
            U64 value = KERNEL_STANDARD_PAGE_FLAGS;
            value |= getZeroedPageTable();
            *tableEntryAddress = value;

            if (!metaData[len - 1]->pages) {
                metaData[len - 1]->pages =
                    (VirtualMetaData *)getZeroedMetaDataTable();
            }
            value = getZeroedMetaData();
            *metaEntryAddress = value;
            metaData[len - 1]->count++;
        }

        pageTables[len] =
            /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
            (VirtualPageTable *)ALIGN_DOWN_VALUE(*tableEntryAddress,
                                                 SMALLEST_VIRTUAL_PAGE);
        metaData[len] = (VirtualMetaData *)*metaEntryAddress;
        len++;
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
    VirtualPageTable *tables[MAX_PAGING_LEVELS];
    tables[0] = rootPageTable;
    U8 len = 1;
    for (U64 pageSize = X86_512GIB_PAGE; pageSize >= SMALLEST_VIRTUAL_PAGE;
         pageSize /= PageTableFormat.ENTRIES) {
        U16 index = calculateTableIndex(virt, pageSize);
        entry = tables[len - 1]->pages[index];

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

        tables[len] =
            /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
            (VirtualPageTable *)ALIGN_DOWN_VALUE(entry, SMALLEST_VIRTUAL_PAGE);
        len++;
    }

    __builtin_unreachable();
}
