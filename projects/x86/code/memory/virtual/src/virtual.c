#include "x86/memory/virtual.h"

#include "abstraction/memory/manipulation.h"
#include "abstraction/memory/physical.h"
#include "shared/assert.h"
#include "shared/maths/maths.h"
#include "shared/memory/converter.h"
#include "shared/memory/management/definitions.h"
#include "shared/memory/management/management.h"
#include "shared/types/numeric.h"
#include "x86/configuration/cpu.h"
#include "x86/memory/definitions.h"
#include "x86/memory/flags.h"

// NOTE: Currently, one should only consult the metadata table through a virtual
// page. I.e., you can only go from page table to meta data.

// static string patEncodingToString[PAT_ENCODING_COUNT] = {
//     STRING("Uncachable (UC)"),        STRING("Write Combining (WC)"),
//     STRING("Reserved 1, don't use!"), STRING("Reserved 2, don't use!"),
//     STRING("Write Through (WT)"),     STRING("Write Protected (WP)"),
//     STRING("Write Back (WB)"),        STRING("Uncached (UC-)"),
// };

VirtualPageTable *rootPageTable;

U16 rootReferenceCount;
VirtualReferenceCount *references;

static U64 getZeroedMemory(U64 bytes, U64 align) {
    U64 address = getBytesForMemoryMapping(bytes, align);
    /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
    memset((void *)address, 0, bytes);
    return address;
}

static U64 getZeroedMetaData() {
    return getZeroedMemory(sizeof(VirtualMetaData), alignof(VirtualMetaData));
}

static constexpr auto META_DATA_TABLE_BYTES =
    PageTableFormat.ENTRIES * sizeof(rootVirtualMetaData);

static U64 getMetaDataTable() {
    U64 address = getBytesForMemoryMapping(META_DATA_TABLE_BYTES,
                                           alignof(typeof(VirtualMetaData)));
    return address;
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

    VirtualPageTable *pageTables[MAX_PAGING_LEVELS];
    pageTables[0] = rootPageTable;
    U64 *tableEntryAddress;

    U8 len = 1;

    for (U64 entrySize = X86_512GIB_PAGE; entrySize >= mappingSize;
         entrySize /= PageTableFormat.ENTRIES) {
        U16 index = calculateTableIndex(virt, entrySize);
        tableEntryAddress = &(pageTables[len - 1]->pages[index]);

        if (entrySize == mappingSize) {
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
                    (VirtualMetaData **)getMetaDataTable();
            }
            metaData[len - 1]->pages[index] =
                (VirtualMetaData *)getZeroedMetaData();
            metaData[len - 1]->count++;
        }

        pageTables[len] =
            /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
            (VirtualPageTable *)ALIGN_DOWN_VALUE(*tableEntryAddress,
                                                 SMALLEST_VIRTUAL_PAGE);
        metaData[len] = metaData[len - 1]->pages[index];
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

static void updateMappingData(VirtualMetaData *metaData[MAX_PAGING_LEVELS],
                              VirtualPageTable *pageTables[MAX_PAGING_LEVELS],
                              U16 indices[MAX_PAGING_LEVELS], U8 len) {
    while (1) {
        metaData[len - 1]->count--;
        if (metaData[len - 1]->count) {
            return;
        }
        pageTables[len - 2]->pages[indices[len - 1]] = 0;
        freePhysicalMemory((Memory){.start = (U64)pageTables[len - 1],
                                    .bytes = VIRTUAL_MEMORY_MAPPING_SIZE});

        metaData[len - 2]->pages[indices[len - 1]] = 0;
        if (metaData[len - 1]->pages) {
            freePhysicalMemory(
                (Memory){.start = (U64)(metaData[len - 1]->pages),
                         .bytes = META_DATA_TABLE_BYTES});
        }
        freePhysicalMemory((Memory){.start = (U64)metaData[len - 1],
                                    .bytes = sizeof(VirtualMetaData)});

        len--;
    }
}

Memory unmapPage(U64 virt) {
    ASSERT(rootPageTable);
    ASSERT(((virt) >> 48L) == 0 || ((virt) >> 48L) == 0xFFFF);

    U16 indices[MAX_PAGING_LEVELS];

    VirtualMetaData *metaData[MAX_PAGING_LEVELS];
    metaData[0] = rootVirtualMetaData;

    U64 tableEntry;
    VirtualPageTable *pageTables[MAX_PAGING_LEVELS];
    pageTables[0] = rootPageTable;

    U8 len = 1;
    for (U64 entrySize = X86_512GIB_PAGE; entrySize >= SMALLEST_VIRTUAL_PAGE;
         entrySize /= PageTableFormat.ENTRIES) {
        U16 index = calculateTableIndex(virt, entrySize);
        indices[len] = index;
        tableEntry = pageTables[len - 1]->pages[index];

        if (!tableEntry || entrySize == SMALLEST_VIRTUAL_PAGE) {
            if (entrySize == SMALLEST_VIRTUAL_PAGE) {
                updateMappingData(metaData, pageTables, indices, len);
            }

            return (Memory){.start = getPhysicalAddressFrame(tableEntry),
                            .bytes = entrySize};
        }

        // NOTE: No possibility of conflicting with bad PATs here because we are
        // not using them.
        if ((tableEntry & VirtualPageMasks.PAGE_EXTENDED_SIZE)) {
            updateMappingData(metaData, pageTables, indices, len);
            return (Memory){.start = getPhysicalAddressFrame(tableEntry),
                            .bytes = entrySize};
        }

        pageTables[len] =
            /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
            (VirtualPageTable *)ALIGN_DOWN_VALUE(tableEntry,
                                                 SMALLEST_VIRTUAL_PAGE);
        metaData[len] = metaData[len - 1]->pages[index];
        len++;
    }

    __builtin_unreachable();
}
