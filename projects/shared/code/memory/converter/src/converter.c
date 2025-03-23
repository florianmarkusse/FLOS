#include "shared/memory/converter.h"

#include "abstraction/memory/virtual/converter.h"
#include "shared/assert.h"
#include "shared/maths/maths.h"
#include "shared/types/types.h"
static bool isPageSizeValid(U64 pageSize) {
    ASSERT(!(RING_RANGE_VALUE(pageSize, pageSize)));
    return pageSize & AVAILABLE_PAGE_SIZES_MASK;
}

static U64 smallestPageSize = 1ULL
                              << __builtin_ctzll(AVAILABLE_PAGE_SIZES_MASK);
static U64 largestPageSize = 1ULL
                             << (((sizeof(U64) * BITS_PER_BYTE) - 1) -
                                 __builtin_clzll(AVAILABLE_PAGE_SIZES_MASK));

Pages convertPreferredPageToAvailablePages(Pages pages) {
    ASSERT(((pages.pageSize) & (pages.pageSize - 1)) == 0);
    if (pages.pageSize < smallestPageSize) {
        return (Pages){.numberOfPages = 1, .pageSize = smallestPageSize};
    }
    while (!isPageSizeValid(pages.pageSize)) {
        pages.numberOfPages *= 2;
        pages.pageSize /= 2;
    }
    return pages;
}

Pages convertBytesToPagesRoundingUp(U64 bytes) {
    // NOTE: We skip the smallest page size in this loop because the check is
    // redundant.
    for (U64 i = MEMORY_PAGE_SIZES_COUNT - 1; i > 0; i--) {
        if (availablePageSizes[i] / 2 <= bytes) {
            Pages result;
            result.pageSize = availablePageSizes[i];
            result.numberOfPages = CEILING_DIV_VALUE(bytes, result.pageSize);
            return result;
        }
    }

    return (Pages){.numberOfPages = CEILING_DIV_VALUE(bytes, smallestPageSize),
                   .pageSize = smallestPageSize};
}

static U64 largestAlignedPage(U64 address) {
    ASSERT(!(RING_RANGE_VALUE(address, smallestPageSize)));

    if (address == 0) {
        return availablePageSizes[MEMORY_PAGE_SIZES_COUNT - 1];
    }

    U64 result = 1 << __builtin_ctzll(address);
    while (!isPageSizeValid(result)) {
        result /= 2;
    }

    return result;
}

U64 static getLargestAlignedPageSize(U64 virt, U64 physical) {
    U64 virtPage = largestAlignedPage(virt);
    U64 physPage = largestAlignedPage(physical);

    return MIN(virtPage, physPage);
}

U64 static getAlignedBytes(U64 bytes) {
    return MIN(MAX(ceilingPowerOf2(bytes), smallestPageSize), largestPageSize);
}

U64 convertToMostFittingAlignedPageSize(U64 virt, U64 physical, U64 bytes) {
    U64 alignedBytes = getAlignedBytes(bytes);
    while (!isPageSizeValid(alignedBytes)) {
        // NOTE: decrease page size because we don't want to go a larger page
        // size than bytes
        alignedBytes /= 2;
    }

    return MIN(getLargestAlignedPageSize(virt, physical), alignedBytes);
}

U64 convertToLargestAlignedPageSize(U64 virt, U64 physical, U64 bytes) {
    U64 alignedBytes = getAlignedBytes(bytes);
    while (!isPageSizeValid(alignedBytes)) {
        // NOTE: increase page size because we can go to a larger page
        // size than bytes
        alignedBytes *= 2;
    }

    return MIN(getLargestAlignedPageSize(virt, physical), alignedBytes);
}
