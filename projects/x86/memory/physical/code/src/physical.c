#include "x86/memory/physical.h"

#include "abstraction/interrupts.h"
#include "abstraction/memory/manipulation.h"
#include "abstraction/memory/physical/allocation.h"
#include "efi-to-kernel/kernel-parameters.h" // for KernelMemory
#include "efi-to-kernel/memory/descriptor.h"
#include "shared/assert.h"
#include "shared/maths/maths.h"
#include "shared/types/types.h" // for U64, U32, U8

// NOTE: This is for an abstraction used in virtual allocation.
U64 allocate4KiBPages(U64 numPages) {
    return allocContiguousPhysicalPages(numPages, BASE_PAGE);
}

PhysicalMemoryManager basePMM;
PhysicalMemoryManager largePMM;
PhysicalMemoryManager hugePMM;

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
    case BASE_PAGE: {
        return &basePMM;
        break;
    }
    case LARGE_PAGE: {
        return &largePMM;
        break;
    }
    case HUGE_PAGE: {
        return &hugePMM;
        break;
    }
    }
}

static U64 memoryEntriesInBasePages(U64 num) {
    return (PAGE_FRAME_SIZE * (num)) / sizeof(PagedMemory);
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

    if (manager->pageSize < HUGE_PAGE) {
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

    if (manager->pageSize < HUGE_PAGE) {
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
    ASSERT(pageType == LARGE_PAGE || pageType == HUGE_PAGE);

    PhysicalMemoryManager *initingManager = getMemoryManager(pageType);
    ASSERT(initingManager->usedBasePages == 1 &&
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

static PagedMemory *nextValidDescriptor(KernelMemory kernelMemory) {
    static U64 i = 0;
    while (i < kernelMemory.memory.len) {
        PagedMemory *result = &kernelMemory.memory.buf[i];
        // Always increment even if found, so the next caller wont get the same
        // descriptor.
        i++;
        return result;
    }

    return nullptr;
}

// Coming into this, All the memory is identity mapped.
// Having to do some boostrapping here with the base page frame physical
// manager.
void initPhysicalMemoryManager(KernelMemory kernelMemory) {
    // Reset the PMMs if they were used previously already
    basePMM =
        (PhysicalMemoryManager){.pageSize = BASE_PAGE, .usedBasePages = 1};
    largePMM =
        (PhysicalMemoryManager){.pageSize = LARGE_PAGE, .usedBasePages = 1};
    hugePMM =
        (PhysicalMemoryManager){.pageSize = HUGE_PAGE, .usedBasePages = 1};

    PagedMemory *descriptor = nextValidDescriptor(kernelMemory);
    if (!descriptor) {
        interruptNoMorePhysicalMemory();
    }

    /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
    basePMM.memory.buf = (PagedMemory *)descriptor->start;
    basePMM.memory.cap = memoryEntriesInBasePages(1);
    basePMM.memory.len = 0;
    descriptor->start += PAGE_FRAME_SIZE;
    descriptor->numberOfPages--;

    if (descriptor->numberOfPages == 0) {
        descriptor = nextValidDescriptor(kernelMemory);
        if (!descriptor) {
            interruptNoMorePhysicalMemory();
        }
    }

    U64 maxCapacity = memoryEntriesInBasePages(descriptor->numberOfPages);
    PagedMemory_a freeMemoryArray =
        (PagedMemory_a){/* NOLINTNEXTLINE(performance-no-int-to-ptr) */
                        .buf = (PagedMemory *)descriptor->start,
                        .len = 0};
    // The memory used to store the free memory is also "free" memory
    // after the PMM is correctly initialized.
    PagedMemory freeMemoryHolder = (PagedMemory){
        .start = descriptor->start, .numberOfPages = descriptor->numberOfPages};

    while ((descriptor = nextValidDescriptor(kernelMemory))) {
        if (freeMemoryArray.len >= maxCapacity) {
            freePhysicalPages(freeMemoryArray, BASE_PAGE);
            freeMemoryArray.len = 0;
        }
        freeMemoryArray.buf[freeMemoryArray.len] =
            (PagedMemory){.start = descriptor->start,
                          .numberOfPages = descriptor->numberOfPages};
        freeMemoryArray.len++;
    }

    freePhysicalPages(freeMemoryArray, BASE_PAGE);
    freePhysicalPage(freeMemoryHolder, BASE_PAGE);

    freePhysicalPage((PagedMemory){.start = (U64)kernelMemory.memory.buf,
                                   .numberOfPages = kernelMemory.pages},
                     BASE_PAGE);

    initPMM(LARGE_PAGE);
    initPMM(HUGE_PAGE);
}
