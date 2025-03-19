#include "x86/memory/physical.h"

#include "abstraction/interrupts.h"
#include "abstraction/memory/manipulation.h"
#include "abstraction/memory/physical/allocation.h"
#include "efi-to-kernel/kernel-parameters.h" // for KernelMemory
#include "shared/assert.h"
#include "shared/maths/maths.h"
#include "shared/types/types.h" // for U64, U32, U8

// NOTE: This is for an abstraction used in virtual allocation.
U64 getPageForMappingVirtualMemory() {
    return allocContiguousPhysicalPages(1, X86_4KIB_PAGE);
}

PhysicalMemoryManager basePMM =
    (PhysicalMemoryManager){.pageSize = X86_4KIB_PAGE};
PhysicalMemoryManager largePMM =
    (PhysicalMemoryManager){.pageSize = X86_2MIB_PAGE};
PhysicalMemoryManager hugePMM =
    (PhysicalMemoryManager){.pageSize = X86_1GIB_PAGE};

static U64 toLargerPages(U64 numberOfPages) {
    return numberOfPages / PageTableFormat.ENTRIES;
}

static U64 toSmallerPages(U64 numberOfPages) {
    return numberOfPages * PageTableFormat.ENTRIES;
}

static PageSize toLargerPageSize(PageSize pageSize) {
    return pageSize * PageTableFormat.ENTRIES;
}

static PageSize toSmallerPageSize(PageSize pageSize) {
    return pageSize / PageTableFormat.ENTRIES;
}

static void decreasePages(PhysicalMemoryManager *manager, U64 index,
                          U64 decreaseBy) {
    ASSERT(manager->memory.buf[index].numberOfPages >= decreaseBy);
    if (manager->memory.buf[index].numberOfPages == decreaseBy) {
        manager->memory.buf[index] =
            manager->memory.buf[manager->memory.len - 1];
        manager->memory.len--;
    } else {
        manager->memory.buf[index].numberOfPages -= decreaseBy;
        manager->memory.buf[index].start += decreaseBy * manager->pageSize;
    }
}

static PhysicalMemoryManager *getMemoryManager(PageSize pageSize) {
    switch (pageSize) {
    case X86_4KIB_PAGE: {
        return &basePMM;
        break;
    }
    case X86_2MIB_PAGE: {
        return &largePMM;
        break;
    }
    case X86_1GIB_PAGE: {
        return &hugePMM;
        break;
    }
    }
}

static U64 memoryEntriesInBasePages(U64 num) {
    return (X86_4KIB_PAGE * (num)) / sizeof(PagedMemory);
}

// NOTE: We can add an index on top of the pages if this function becomes an
// issue that is based on available pages.
static U64
allocContiguousPhysicalPagesWithManager(U64 numberOfPages,
                                        PhysicalMemoryManager *manager) {
    for (U32 i = 0; i < manager->memory.len; i++) {
        if (manager->memory.buf[i].numberOfPages >= numberOfPages) {
            U64 address = manager->memory.buf[i].start;
            decreasePages(manager, i, numberOfPages);
            return address;
        }
    }

    if (manager->pageSize < X86_1GIB_PAGE) {
        U64 pagesForLargerManager =
            CEILING_DIV_VALUE(numberOfPages, PageTableFormat.ENTRIES);
        U64 address = allocContiguousPhysicalPages(
            pagesForLargerManager, toLargerPageSize(manager->pageSize));

        U64 freeAddressStart = address + numberOfPages * manager->pageSize;
        U64 freePages = toSmallerPages(pagesForLargerManager) - numberOfPages;

        freePhysicalPage((PagedMemory){.numberOfPages = freePages,
                                       .start = freeAddressStart},
                         manager->pageSize);

        return address;
    }

    interruptNoMorePhysicalMemory();
}

U64 allocContiguousPhysicalPages(U64 numberOfPages, PageSize pageSize) {
    return allocContiguousPhysicalPagesWithManager(numberOfPages,
                                                   getMemoryManager(pageSize));
}

static PagedMemory_a
allocPhysicalPagesWithManager(PagedMemory_a pages,
                              PhysicalMemoryManager *manager) {
    U32 requestedPages = (U32)pages.len;

    // The final output array may have fewer entries since the memory might be
    // in contiguous blocks.
    pages.len = 0;
    for (U64 i = manager->memory.len - 1; manager->memory.len > 0;
         manager->memory.len--, i = manager->memory.len - 1) {
        if (manager->memory.buf[i].numberOfPages >= requestedPages) {
            pages.buf[pages.len].numberOfPages = requestedPages;
            pages.buf[pages.len].start = manager->memory.buf[i].start;
            pages.len++;

            decreasePages(manager, i, requestedPages);
            return pages;
        }

        pages.buf[pages.len] = manager->memory.buf[i];
        pages.len++;
        requestedPages -= manager->memory.buf[i].numberOfPages;
    }

    if (manager->pageSize < X86_1GIB_PAGE) {
        // Convert to larger manager request
        // Can slyly use the leftover buffer as it is assumed that it is at
        // least big enough for the smaller buffer so definitely true for the
        // larger buffer
        PagedMemory_a leftOverRequest = (PagedMemory_a){
            .buf = pages.buf + pages.len,
            .len = CEILING_DIV_VALUE(requestedPages, PageTableFormat.ENTRIES)};

        PagedMemory_a largerPage = allocPhysicalPages(
            leftOverRequest, toLargerPageSize(manager->pageSize));
        // Convert back to original manager sizes
        for (U64 i = 0; i < largerPage.len; i++) {
            largerPage.buf[i].numberOfPages *= PageTableFormat.ENTRIES;
        }
        freePhysicalPages(largerPage, manager->pageSize);

        // The actual leftover request can now be satisfied
        leftOverRequest.len = requestedPages;
        PagedMemory_a addedPages =
            allocPhysicalPagesWithManager(leftOverRequest, manager);

        // "Concat" both arrays, the original found pages and those available
        // after stealing from big brother
        pages.len += addedPages.len;
        return pages;
    }

    interruptNoMorePhysicalMemory();
}

PagedMemory_a allocPhysicalPages(PagedMemory_a pages, PageSize pageSize) {
    return allocPhysicalPagesWithManager(pages, getMemoryManager(pageSize));
}

static void freePhysicalPagesWithManager(PagedMemory_a pages,
                                         PhysicalMemoryManager *manager) {
    for (U64 i = 0; i < pages.len; i++) {
        if (manager->memory.len >= manager->memory.cap) {
            PagedMemory *newBuf =
                /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
                (PagedMemory *)allocContiguousPhysicalPagesWithManager(
                    manager->usedBasePages + 1, &basePMM);
            memcpy(newBuf, manager->memory.buf,
                   manager->memory.len * sizeof(*manager->memory.buf));
            // The page that just got freed should be added now.
            newBuf[manager->memory.len] =
                (PagedMemory){.start = (U64)manager->memory.buf,
                              .numberOfPages = manager->usedBasePages};
            manager->memory.len++;
            manager->memory.buf = newBuf;
            (manager->usedBasePages)++;
            manager->memory.cap =
                memoryEntriesInBasePages(manager->usedBasePages);
        }
        manager->memory.buf[manager->memory.len] = pages.buf[i];
        manager->memory.len++;
    }
}

void freePhysicalPages(PagedMemory_a page, PageSize pageSize) {
    return freePhysicalPagesWithManager(page, getMemoryManager(pageSize));
}

void freePhysicalPage(PagedMemory page, PageSize pageSize) {
    return freePhysicalPagesWithManager(
        (PagedMemory_a){.len = 1, .buf = (PagedMemory[]){page}},
        getMemoryManager(pageSize));
}

static void initPMM(PageSize pageType) {
    ASSERT(pageType == X86_2MIB_PAGE || pageType == X86_1GIB_PAGE);

    PhysicalMemoryManager *initingManager = getMemoryManager(pageType);
    ASSERT(initingManager->usedBasePages == 0 &&
           initingManager->memory.len == 0);

    initingManager->memory.buf =
        /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
        (PagedMemory *)allocContiguousPhysicalPagesWithManager(
            initingManager->usedBasePages, &basePMM);
    initingManager->memory.cap =
        memoryEntriesInBasePages(initingManager->usedBasePages);

    //  12 For the aligned page frame always
    //  9 for every level of page table (large and huge currently)
    U64 initedManagerPageSize = toSmallerPageSize(pageType);
    PhysicalMemoryManager *initedManager =
        getMemoryManager(initedManagerPageSize);
    U64 initingManagerPageSize = pageType;

    for (U64 i = 0; i < initedManager->memory.len; i++) {
        PagedMemory memory = initedManager->memory.buf[i];

        if (memory.numberOfPages >= PageTableFormat.ENTRIES) {
            U64 applicablePageBoundary =
                ALIGN_UP_VALUE(memory.start, initingManagerPageSize);
            U64 pagesMoved =
                (applicablePageBoundary - memory.start) / initedManagerPageSize;
            U64 pagesFromAlign = memory.numberOfPages - pagesMoved;
            // Now we are actually able to move pages into the PMM at the higher
            // level.
            if (pagesFromAlign >= PageTableFormat.ENTRIES) {
                decreasePages(initedManager, i, pagesFromAlign);
                // We may have 600 free pages that start on the aligned
                // boundary, but the size of the next level of physical memory
                // manager is 512 * current level, so we can only "upgrade"
                // every 512 pages to 1 new page. The rest is leftover and will
                // be added back to the current level.
                U64 alignedForNextLevelPages =
                    ALIGN_DOWN_VALUE(pagesFromAlign, PageTableFormat.ENTRIES);
                freePhysicalPage((PagedMemory){.start = applicablePageBoundary,
                                               .numberOfPages = (toLargerPages(
                                                   alignedForNextLevelPages))},
                                 pageType);
                U64 leftoverPages = pagesFromAlign - alignedForNextLevelPages;
                if (leftoverPages > 0) {
                    freePhysicalPage(
                        (PagedMemory){.start = applicablePageBoundary +
                                               (alignedForNextLevelPages *
                                                initedManagerPageSize),
                                      .numberOfPages = leftoverPages},
                        initedManagerPageSize);
                }
            }
        }
    }
}

// Coming into this, All the memory is identity mapped.
// Having to do some boostrapping here with the base page frame physical
// manager.
void initPhysicalMemoryManager(KernelMemory kernelMemory) {
    // Reset the PMMs if they were used previously already
    basePMM =
        (PhysicalMemoryManager){.pageSize = X86_4KIB_PAGE,
                                .usedBasePages = (U32)CEILING_DIV_VALUE(
                                    kernelMemory.UEFIPages, X86_4KIB_PAGE)};
    basePMM.memory.buf = kernelMemory.memory.buf;
    basePMM.memory.len = kernelMemory.memory.len;
    basePMM.memory.cap = memoryEntriesInBasePages(basePMM.usedBasePages);

    largePMM.usedBasePages = 0;
    hugePMM.usedBasePages = 0;

    initPMM(X86_2MIB_PAGE);
    initPMM(X86_1GIB_PAGE);
}
