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
PageMetaDataNode rootPageMetaData = {0};

static U64 getZeroedMemory(U64 bytes, U64 align) {
    U64 address = getBytesForMemoryMapping(bytes, align);
    /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
    memset((void *)address, 0, bytes);
    return address;
}

static constexpr auto NEW_META_DATA_TABLE_BYTES =
    PageTableFormat.ENTRIES * sizeof(PageMetaDataNode);
static PageMetaDataNode *getNewMetaDataTable() {
    U64 address = getBytesForMemoryMapping(NEW_META_DATA_TABLE_BYTES,
                                           alignof(PageMetaDataNode));
    return (PageMetaDataNode *)address;
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

    PageMetaDataNode *metaDataTable = &rootPageMetaData;
    VirtualPageTable *pageTable = rootPageTable;

    U16 index = 0;
    for (U64 entrySize = X86_512GIB_PAGE; entrySize >= mappingSize;
         entrySize /= PageTableFormat.ENTRIES) {
        U16 newMetaIndex = index; // Lagging behind index by 1 iteration
        index = calculateTableIndex(virt, entrySize);

        U64 *tableEntryAddress = &(pageTable->pages[index]);
        PageMetaDataNode *newMetaEntryAddress = &(metaDataTable[newMetaIndex]);

        if (entrySize == mappingSize) {
            U64 value = physical | flags;
            if (mappingSize == X86_2MIB_PAGE || mappingSize == X86_1GIB_PAGE) {
                value |= VirtualPageMasks.PAGE_EXTENDED_SIZE;
            }
            *tableEntryAddress = value;
            newMetaEntryAddress->metaData.entriesMapped++;
            return;
        }

        if (!(*tableEntryAddress)) {
            *tableEntryAddress = getZeroedPageTable() | STANDARD_PAGE_FLAGS;

            newMetaEntryAddress->metaData.entriesMapped++;
            newMetaEntryAddress->metaData.entriesMappedWithSmallerGranularity++;
            if (!(newMetaEntryAddress->children)) {
                newMetaEntryAddress->children = getNewMetaDataTable();
            }
        }

        pageTable =
            /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
            (VirtualPageTable *)ALIGN_DOWN_VALUE(*tableEntryAddress,
                                                 SMALLEST_VIRTUAL_PAGE);
        metaDataTable = newMetaEntryAddress->children;
    }
}

void mapPage(U64 virt, U64 physical, U64 mappingSize) {
    return mapPageWithFlags(virt, physical, mappingSize, STANDARD_PAGE_FLAGS);
}

U64 getPhysicalAddressFrame(U64 virtualPage) {
    return virtualPage & VirtualPageMasks.FRAME_OR_NEXT_PAGE_TABLE;
}

static void updateMappingData(VirtualPageTable *pageTables[MAX_PAGING_LEVELS],
                              U16 pageTableIndices[MAX_PAGING_LEVELS],
                              PageMetaDataNode *metaData[MAX_PAGING_LEVELS],
                              U16 metaDataTableIndices[MAX_PAGING_LEVELS],
                              U8 len) {
    while (1) {
        pageTables[len - 1]->pages[pageTableIndices[len - 1]] = 0;
        metaData[len - 1][metaDataTableIndices[len - 1]]
            .metaData.entriesMapped--;

        if (metaData[len - 1][metaDataTableIndices[len - 1]]
                .metaData.entriesMapped) {
            return;
        }

        metaData[len - 2][metaDataTableIndices[len - 2]]
            .metaData.entriesMappedWithSmallerGranularity--;

        if (!(metaData[len - 2][metaDataTableIndices[len - 2]]
                  .metaData.entriesMappedWithSmallerGranularity)) {
            metaData[len - 2][metaDataTableIndices[len - 2]].children = 0;
            freePhysicalMemory((Memory){.start = (U64)(metaData[len - 1]),
                                        .bytes = NEW_META_DATA_TABLE_BYTES});
        }

        freePhysicalMemory((Memory){.start = (U64)pageTables[len - 1],
                                    .bytes = VIRTUAL_MEMORY_MAPPING_SIZE});

        len--;
    }
}

Memory unmapPage(U64 virt) {
    ASSERT(rootPageTable);
    ASSERT(((virt) >> 48L) == 0 || ((virt) >> 48L) == 0xFFFF);

    U16 indices[MAX_PAGING_LEVELS +
                1]; // Counts all the indices, including the first 0 "index"
                    // from the metadata index. So,
                    // - metaIndices = [0, MAX_PAGING_LEVELS],
                    // - pageTableIndices = [1, MAX_PAGING_LEVELS + 1]
    indices[0] = 0;

    PageMetaDataNode *newMeta[MAX_PAGING_LEVELS];
    newMeta[0] = &rootPageMetaData;

    VirtualPageTable *pageTables[MAX_PAGING_LEVELS];
    pageTables[0] = rootPageTable;

    U8 len = 1;
    U16 index = 0;
    for (U64 entrySize = X86_512GIB_PAGE; entrySize >= SMALLEST_VIRTUAL_PAGE;
         entrySize /= PageTableFormat.ENTRIES) {
        U16 newMetaIndex = index; // Lagging behind index by 1 iteration
        index = calculateTableIndex(virt, entrySize);

        indices[len] = index;
        U64 tableEntry = pageTables[len - 1]->pages[index];

        if (!tableEntry || entrySize == SMALLEST_VIRTUAL_PAGE) {
            if (tableEntry) {
                updateMappingData(pageTables, indices + 1, newMeta, indices,
                                  len);
            }

            return (Memory){.start = getPhysicalAddressFrame(tableEntry),
                            .bytes = entrySize};
        }

        // NOTE: No possibility of conflicting with bad PATs here because we are
        // not using them.
        if ((tableEntry & VirtualPageMasks.PAGE_EXTENDED_SIZE)) {
            updateMappingData(pageTables, indices + 1, newMeta, indices, len);
            return (Memory){.start = getPhysicalAddressFrame(tableEntry),
                            .bytes = entrySize};
        }

        pageTables[len] =
            /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
            (VirtualPageTable *)ALIGN_DOWN_VALUE(tableEntry,
                                                 SMALLEST_VIRTUAL_PAGE);
        newMeta[len] = newMeta[len - 1][newMetaIndex].children;
        len++;
    }

    __builtin_unreachable();
}
