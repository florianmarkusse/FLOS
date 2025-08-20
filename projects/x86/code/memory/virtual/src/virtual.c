#include "x86/memory/virtual.h"

#include "abstraction/memory/manipulation.h"
#include "abstraction/memory/virtual/allocator.h"
#include "abstraction/memory/virtual/converter.h"
#include "shared/assert.h"
#include "shared/maths.h"
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

StructReq virtualStructReqs[VIRTUAL_ALLOCATION_TYPE_COUNT] = {
    [VIRTUAL_PAGE_TABLE_ALLOCATION] = {.bytes = X86_4KIB_PAGE,
                                       .align = X86_4KIB_PAGE},
    [META_DATA_PAGE_ALLOCATION] = {.bytes = PageTableFormat.ENTRIES *
                                            sizeof(rootPageMetaData),
                                   .align = alignof(rootPageMetaData)}

};

VirtualPageTable *getZeroedPageTable() {
    return getZeroedMemoryForVirtual(VIRTUAL_PAGE_TABLE_ALLOCATION);
}

static PageMetaDataNode *getZeroedMetaDataTable() {
    return getZeroedMemoryForVirtual(META_DATA_PAGE_ALLOCATION);
}

static U16 calculateTableIndex(U64 virt, U64_pow2 pageSize) {
    return (U16)ringBufferIndex(divideByPowerOf2(virt, pageSize),
                                PageTableFormat.ENTRIES);
}

void mapPage_(U64 virt, U64 physical, U64_pow2 mappingSize, U64 flags) {
    ASSERT(rootPageTable);
    ASSERT(!(ringBufferIndex(physical, mappingSize)));
    ASSERT(isPowerOf2(mappingSize));

    PageMetaDataNode *metaDataTable = &rootPageMetaData;
    VirtualPageTable *pageTable = rootPageTable;

    U16 index = 0;
    for (typeof(mappingSize) entrySize = PAGE_ROOT_ENTRY_MAX_SIZE;
         entrySize >= mappingSize; entrySize /= PageTableFormat.ENTRIES) {
        U16 newMetaIndex = index; // Lagging behind index by 1 iteration
        index = calculateTableIndex(virt, entrySize);

        U64 *tableEntryAddress = &(pageTable->pages[index]);
        PageMetaDataNode *newMetaEntryAddress = &(metaDataTable[newMetaIndex]);

        if (entrySize == mappingSize) {
            U64 value = physical | flags;
            if (mappingSize & (X86_2MIB_PAGE | X86_1GIB_PAGE)) {
                value |= VirtualPageMasks.PAGE_EXTENDED_SIZE;
            }
            *tableEntryAddress = value;
            newMetaEntryAddress->metaData.entriesMapped++;
            return;
        }

        if (!(*tableEntryAddress)) {
            *tableEntryAddress =
                (U64)getZeroedPageTable() | pageFlagsReadWrite();

            newMetaEntryAddress->metaData.entriesMapped++;
            newMetaEntryAddress->metaData.entriesMappedWithSmallerGranularity++;
            if (!(newMetaEntryAddress->children)) {
                newMetaEntryAddress->children = getZeroedMetaDataTable();
            }
        }

        pageTable =
            /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
            (VirtualPageTable *)alignDown(*tableEntryAddress,
                                          pageSizesSmallest());
        metaDataTable = newMetaEntryAddress->children;
    }
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

        // Will decrement entriesMapped in the next iteration of the loop.
        metaData[len - 2][metaDataTableIndices[len - 2]]
            .metaData.entriesMappedWithSmallerGranularity--;

        if (!(metaData[len - 2][metaDataTableIndices[len - 2]]
                  .metaData.entriesMappedWithSmallerGranularity)) {
            metaData[len - 2][metaDataTableIndices[len - 2]].children = 0;
            freeZeroedMemoryForVirtual((U64)(metaData[len - 1]),
                                       META_DATA_PAGE_ALLOCATION);
        }

        freeZeroedMemoryForVirtual((U64)pageTables[len - 1],
                                   VIRTUAL_PAGE_TABLE_ALLOCATION);
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
    for (U64_pow2 entrySize = X86_512GIB_PAGE; entrySize >= pageSizesSmallest();
         entrySize /= PageTableFormat.ENTRIES) {
        U16 newMetaIndex = index; // Lagging behind index by 1 iteration
        index = calculateTableIndex(virt, entrySize);

        indices[len] = index;
        U64 tableEntry = pageTables[len - 1]->pages[index];

        if (!tableEntry || entrySize == pageSizesSmallest()) {
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
            (VirtualPageTable *)alignDown(tableEntry, pageSizesSmallest());
        newMeta[len] = newMeta[len - 1][newMetaIndex].children;
        len++;
    }

    __builtin_unreachable();
}
