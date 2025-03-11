#include "abstraction/memory/management/policy.h"

#include "abstraction/interrupts.h"
#include "abstraction/memory/virtual/map.h"
#include "shared/maths/maths.h"
#include "shared/memory/converter.h"
#include "shared/memory/management/definitions.h"
#include "shared/types/types.h"
#include "x86/memory/definitions.h"
#include "x86/memory/physical.h"
#include "x86/memory/policy/virtual.h"

static U64 allocateVirtualMemory(Pages pages) {
    U64 size = pages.numberOfPages * pages.pageSize;
    return getVirtualMemory(size, pages.pageSize);
}

// NOTE: It would be weird if someone asked for 300 pages of 16 MiB, how did
// you get to this number? I don't see why supporting this is necessary.
static void ensureNormalNuberOfPages(U64 numberOfPages) {
    if (numberOfPages > PageTableFormat.ENTRIES) {
        interruptTooLargeAllocation();
    }
}

static void *getVirtualAndPhysicalAndMap(Pages pages) {
    ensureNormalNuberOfPages(pages.numberOfPages);
    U64 virtualAddress = allocateVirtualMemory(pages);

    PagedMemory pagedMemory[PageTableFormat.ENTRIES];
    PagedMemory_a request =
        (PagedMemory_a){.buf = pagedMemory, .len = pages.numberOfPages};
    PagedMemory_a physicalAddresses =
        allocPhysicalPages(request, pages.pageSize);
    U64 virtualRegion = virtualAddress;
    for (U64 i = 0; i < physicalAddresses.len; i++) {
        mapVirtualRegion(virtualRegion, physicalAddresses.buf[i],
                         pages.pageSize);
        virtualRegion +=
            physicalAddresses.buf[i].numberOfPages * pages.pageSize;
    }

    /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
    return (void *)virtualAddress;
}

void *allocAndMapExplicit(Pages pages) {
    pages = convertPreferredPageToAvailablePages(pages);
    return getVirtualAndPhysicalAndMap(pages);
}

void *allocAndMap(U64 bytes) {
    Pages pages = convertBytesToPagesRoundingUp(bytes);
    return getVirtualAndPhysicalAndMap(pages);
}

void *allocContiguousAndMap(Pages pages) {
    pages = convertPreferredPageToAvailablePages(pages);
    ensureNormalNuberOfPages(pages.numberOfPages);
    U64 virtualAddress = allocateVirtualMemory(pages);

    U64 physicalAddress =
        allocContiguousPhysicalPages(pages.numberOfPages, pages.pageSize);
    mapVirtualRegion(virtualAddress,
                     (PagedMemory){.numberOfPages = pages.numberOfPages,
                                   .start = physicalAddress},
                     pages.pageSize);

    /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
    return (void *)virtualAddress;
}

void freeMapped(U64 start, U64 bytes) {
    U64 end = start + bytes;

    MappedPage page = getMappedPage(start);
    if (RING_RANGE_VALUE(start, page.pageSize)) {
        start = ALIGN_UP_VALUE(start, page.pageSize);
        if (start >= end) {
            return;
        }
        page = getMappedPage(start);
    }

    U64 physicalOfPage = getPhysicalAddressFrame(page.entry.value);

    PagedMemory pagedEntry =
        (PagedMemory){.start = physicalOfPage, .numberOfPages = 1};

    PageSize previousPageSize = page.pageSize;
    U64 nextPhysical = physicalOfPage + previousPageSize;

    start += page.pageSize;
    while (start < end) {
        page = getMappedPage(start);
        physicalOfPage = getPhysicalAddressFrame(page.entry.value);

        if (physicalOfPage == nextPhysical &&
            previousPageSize == page.pageSize) {
            pagedEntry.numberOfPages++;
            nextPhysical += previousPageSize;
        } else {
            freePhysicalPage(pagedEntry, previousPageSize);

            pagedEntry =
                (PagedMemory){.start = physicalOfPage, .numberOfPages = 1};

            previousPageSize = page.pageSize;
            nextPhysical = physicalOfPage + previousPageSize;
        }

        start += page.pageSize;
    }

    freePhysicalPage(pagedEntry, previousPageSize);
}
