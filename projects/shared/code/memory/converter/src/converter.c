#include "shared/memory/converter.h"

#include "abstraction/memory/virtual/converter.h"
#include "shared/assert.h"
#include "shared/maths/maths.h"
#include "shared/types/types.h"
static bool isPageSizeValid(U64 pageSize) {
    ASSERT(!(RING_RANGE_VALUE(pageSize, pageSize)));
    return pageSize & AVAILABLE_PAGE_SIZES_MASK;
}

Pages convertPreferredPageToAvailablePages(Pages pages) {
    ASSERT(((pages.pageSize) & (pages.pageSize - 1)) == 0);
    if (pages.pageSize < SMALLEST_VIRTUAL_PAGE) {
        return (Pages){.numberOfPages = 1, .pageSize = SMALLEST_VIRTUAL_PAGE};
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

    return (Pages){.numberOfPages =
                       CEILING_DIV_VALUE(bytes, SMALLEST_VIRTUAL_PAGE),
                   .pageSize = SMALLEST_VIRTUAL_PAGE};
}

U64 static pageAligned(U64 bytes) {
    return MIN(MAX(ceilingPowerOf2(bytes), SMALLEST_VIRTUAL_PAGE),
               LARGEST_VIRTUAL_PAGE);
}

U64 increasePageSize(U64 pageSize) {
    U64 largerPages =
        (~((pageSize | (pageSize - 1)))) & AVAILABLE_PAGE_SIZES_MASK;

    return largerPages & -largerPages;
}

U64 decreasePageSize(U64 pageSize) {
    U64 smallerPages = ((pageSize - 1) & AVAILABLE_PAGE_SIZES_MASK);
    return 1ULL << (((sizeof(U64) * BITS_PER_BYTE) - 1) -
                    (__builtin_clzll(smallerPages)));
}

U64 pageSizeEncompassing(U64 bytes) {
    U64 result = pageAligned(bytes);

    while (!isPageSizeValid(result)) {
        result *= 2;
    }

    return result;
}

static U64 largestAlignedPage(U64 address) {
    ASSERT(!(RING_RANGE_VALUE(address, SMALLEST_VIRTUAL_PAGE)));

    if (address == 0) {
        return LARGEST_VIRTUAL_PAGE;
    }

    U64 result = (1ULL << __builtin_ctzll(address));

    while (!isPageSizeValid(result)) {
        result /= 2;
    }

    return result;
}

// NOTE: ready for code generation
// Only difference is multiplication / division
U64 pageSizeLeastLargerThan(U64 address, U64 bytes) {
    U64 maxPageSize = largestAlignedPage(address);
    U64 alignedBytes = pageAligned(bytes);

    if (alignedBytes > maxPageSize) {
        return maxPageSize;
    }

    while (!isPageSizeValid(alignedBytes)) {
        alignedBytes *= 2;
    }

    return alignedBytes;
}

// NOTE: ready for code generation
U64 pageSizeFitting(U64 address, U64 bytes) {
    U64 maxPageSize = largestAlignedPage(address);
    U64 alignedBytes = pageAligned(bytes);

    if (alignedBytes > maxPageSize) {
        return maxPageSize;
    }

    while (!isPageSizeValid(alignedBytes)) {
        alignedBytes /= 2;
    }

    return alignedBytes;
}

U64 static getLargestAlignedPageSize(U64 virt, U64 physical) {
    U64 virtPage = largestAlignedPage(virt);
    U64 physPage = largestAlignedPage(physical);

    return MIN(virtPage, physPage);
}

U64 convertToMostFittingAlignedPageSize(U64 virt, U64 physical, U64 bytes) {
    U64 alignedBytes = pageAligned(bytes);
    while (!isPageSizeValid(alignedBytes)) {
        // NOTE: decrease page size because we don't want to go a larger page
        // size than bytes
        alignedBytes /= 2;
    }

    return MIN(getLargestAlignedPageSize(virt, physical), alignedBytes);
}

U64 convertToLargestAlignedPageSize(U64 virt, U64 physical, U64 bytes) {
    U64 alignedBytes = pageAligned(bytes);

    while (!isPageSizeValid(alignedBytes)) {
        // NOTE: increase page size because we can go to a larger page
        // size than bytes
        alignedBytes *= 2;
    }

    return MIN(getLargestAlignedPageSize(virt, physical), alignedBytes);
}
