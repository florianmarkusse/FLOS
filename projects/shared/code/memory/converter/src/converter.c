#include "shared/memory/converter.h"

#include "abstraction/memory/virtual/converter.h"
#include "shared/assert.h"
#include "shared/maths/maths.h"
#include "shared/types/types.h"
static bool isPageSizeValid(U64 pageSize) {
    ASSERT(((pageSize) & (pageSize - 1)) == 0);
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

Pages convertBytesToSmallestNuberOfPages(U64 bytes) {
    // NOTE: We skip the largest page size in this loop because the check is
    // redundant.
    for (U64 i = 0; i < MEMORY_PAGE_SIZES_COUNT - 1; i++) {
        if (availablePageSizes[i] >= bytes) {
            return (Pages){.pageSize = availablePageSizes[i],
                           .numberOfPages = 1};
        }
    }

    Pages result;
    result.pageSize = largestPageSize;
    result.numberOfPages = CEILING_DIV_VALUE(bytes, result.pageSize);
    return result;
}

bool isValidPageSizeForArch(U64 pageSize) {
    for (U64 i = 0; i < MEMORY_PAGE_SIZES_COUNT; i++) {
        if (availablePageSizes[i] == pageSize) {
            return true;
        }
    }

    return false;
}
