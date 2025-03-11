#include "shared/memory/converter.h"

#include "abstraction/memory/virtual/converter.h"
#include "shared/assert.h"
#include "shared/maths/maths.h"
#include "shared/types/types.h"
static bool isPageSizeValid(U64 pageSize) {
    ASSERT(((pageSize) & (pageSize - 1)) == 0);
    return pageSize & AVAILABLE_PAGE_SIZES_MASK;
}

U64 smallestPageSize = 1 << __builtin_ctzl(AVAILABLE_PAGE_SIZES_MASK);

Pages convertPreferredPageToAvailablePages(Pages pages) {
    ASSERT(((pages.pageSize) & (pages.pageSize - 1)) == 0);
    if (pages.pageSize <= smallestPageSize) {
        return (Pages){.numberOfPages = 1, .pageSize = smallestPageSize};
    }
    while (!isPageSizeValid(pages.pageSize)) {
        pages.numberOfPages <<= 1;
        pages.pageSize >>= 1;
    }
    return pages;
}

Pages convertBytesToPagesRoundingUp(U64 bytes) {
    if (bytes <= smallestPageSize) {
        return (Pages){.numberOfPages = 1, .pageSize = smallestPageSize};
    }

    Pages result;
    for (U64 i = MEMORY_PAGE_SIZES_COUNT - 1; i != U64_MAX; i--) {
        if (pageSizes[i] / 2 <= bytes) {
            result.pageSize = pageSizes[i];
            result.numberOfPages = CEILING_DIV_VALUE(bytes, result.pageSize);
            return result;
        }
    }

    __builtin_unreachable();
}

bool isValidPageSizeForArch(U64 pageSize) {
    for (U64 i = 0; i < MEMORY_PAGE_SIZES_COUNT; i++) {
        if (pageSizes[i] == pageSize) {
            return true;
        }
    }

    return false;
}
