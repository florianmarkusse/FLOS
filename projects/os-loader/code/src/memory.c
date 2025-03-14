#include "os-loader/memory.h"

#include "abstraction/log.h"
#include "abstraction/memory/manipulation.h"
#include "abstraction/memory/physical/allocation.h"
#include "abstraction/memory/virtual/converter.h"
#include "abstraction/memory/virtual/map.h"
#include "efi/error.h"
#include "efi/firmware/system.h"
#include "efi/globals.h"
#include "efi/memory.h"
#include "shared/log.h"
#include "shared/maths/maths.h"
#include "shared/memory/converter.h"
#include "shared/text/string.h"

// TODO: We should really "free" the memory we use for getting memory info...
MemoryInfo getMemoryInfo() {
    MemoryInfo mmap = {0};

    // Call GetMemoryMap with initial buffer size of 0 to retrieve the
    // required buffer size
    Status status = globals.st->boot_services->get_memory_map(
        &mmap.memoryMapSize, mmap.memoryMap, &mmap.mapKey, &mmap.descriptorSize,
        &mmap.descriptorVersion);
    if (status != BUFFER_TOO_SMALL) {
        EXIT_WITH_MESSAGE {
            ERROR(STRING(
                "Should have received a buffer too small error here!\n"));
        }
    }

    mmap.memoryMap = (MemoryDescriptor *)allocate4KiBPages(
        CEILING_DIV_VALUE(mmap.memoryMapSize, UEFI_PAGE_SIZE));

    status = globals.st->boot_services->get_memory_map(
        &mmap.memoryMapSize, mmap.memoryMap, &mmap.mapKey, &mmap.descriptorSize,
        &mmap.descriptorVersion);
    EXIT_WITH_MESSAGE_IF(status) {
        ERROR(STRING("Getting memory map failed!\n"));
    }

    return mmap;
}

static bool canBeUsedByOS(MemoryType type) {
    switch (type) {
    case LOADER_CODE:
    case LOADER_DATA:
    case BOOT_SERVICES_CODE:
    case BOOT_SERVICES_DATA:
    case CONVENTIONAL_MEMORY:
    case PERSISTENT_MEMORY:
        return true;
    default:
        return false;
    }
}

static bool canBeUsedInEFI(MemoryType type) {
    return type == CONVENTIONAL_MEMORY;
}

static constexpr auto MAX_ALLOWED_ALIGNEMT = 1 * GiB;
static constexpr auto MIN_POSSIBLE_ALIGNMENT = UEFI_PAGE_SIZE;
U64 getPhysicalMemory(U64 bytes, U64 alignment) {
    ASSERT(((alignment) & (alignment - 1)) == 0);
    if (alignment > MAX_ALLOWED_ALIGNEMT) {
        EXIT_WITH_MESSAGE {
            ERROR(STRING("alignment exceeded maximum allowed alignment.\n"));
            ERROR(STRING("Request alignment: "));
            ERROR(alignment, NEWLINE);
            ERROR(STRING("Maximum allowed alignment: "));
            ERROR(MAX_ALLOWED_ALIGNEMT, NEWLINE);
        }
    }

    MemoryInfo memoryInfo = {0};

    // Call GetMemoryMap with initial buffer size of 0 to retrieve the
    // required buffer size
    Status status = globals.st->boot_services->get_memory_map(
        &memoryInfo.memoryMapSize, memoryInfo.memoryMap, &memoryInfo.mapKey,
        &memoryInfo.descriptorSize, &memoryInfo.descriptorVersion);
    if (status != BUFFER_TOO_SMALL) {
        EXIT_WITH_MESSAGE {
            ERROR(STRING(
                "Should have received a buffer too small error here!\n"));
        }
    }
    // NOTE: Adding an extra page here to acoount for allocation below
    memoryInfo.memoryMapSize += UEFI_PAGE_SIZE;

    status = globals.st->boot_services->allocate_pages(
        ALLOCATE_ANY_PAGES, LOADER_DATA,
        CEILING_DIV_VALUE(memoryInfo.memoryMapSize, UEFI_PAGE_SIZE),
        (PhysicalAddress *)&memoryInfo.memoryMap);
    EXIT_WITH_MESSAGE_IF(status) {
        ERROR(STRING("allocating pages for getting memory map failed!\n"));
    }

    status = globals.st->boot_services->get_memory_map(
        &memoryInfo.memoryMapSize, memoryInfo.memoryMap, &memoryInfo.mapKey,
        &memoryInfo.descriptorSize, &memoryInfo.descriptorVersion);
    EXIT_WITH_MESSAGE_IF(status) {
        ERROR(STRING("Getting memory map failed!\n"));
    }

    for (U64 largerAlignment = (MAX(alignment, MIN_POSSIBLE_ALIGNMENT) << 1);
         largerAlignment != 0; largerAlignment <<= 1) {
        for (U64 i = 0;
             i < memoryInfo.memoryMapSize / memoryInfo.descriptorSize; i++) {
            MemoryDescriptor *desc =
                (MemoryDescriptor *)((U8 *)memoryInfo.memoryMap +
                                     (i * memoryInfo.descriptorSize));
            if (!canBeUsedInEFI(desc->type)) {
                continue;
            }

            if (RING_RANGE_VALUE(desc->physicalStart, alignment) ||
                !RING_RANGE_VALUE(desc->physicalStart, largerAlignment)) {
                continue;
            }

            if (desc->numberOfPages * UEFI_PAGE_SIZE < bytes) {
                continue;
            }

            U64 address = desc->physicalStart;
            status = globals.st->boot_services->allocate_pages(
                ALLOCATE_ADDRESS, LOADER_DATA,
                CEILING_DIV_VALUE(bytes, UEFI_PAGE_SIZE), &address);
            EXIT_WITH_MESSAGE_IF(status) {
                ERROR(STRING("allocating pages for memory failed!\n"));
            }

            status = globals.st->boot_services->free_pages(
                (U64)memoryInfo.memoryMap,
                CEILING_DIV_VALUE(memoryInfo.memoryMapSize, UEFI_PAGE_SIZE));
            EXIT_WITH_MESSAGE_IF(status) {
                ERROR(STRING(
                    "Freeing allocated memory for memory map failed!\n"));
            }

            return address;
        }
    }

    EXIT_WITH_MESSAGE { ERROR(STRING("Could not find memory!")); }

    __builtin_unreachable();
}

static constexpr auto ADDITIONAL_CAPACITY_FOR_SPLITTING_MEMORY_DESCRIPTOR = 1;
KernelMemory stubMemoryBeforeExitBootServices(MemoryInfo *memoryInfo) {
    KernelMemory result;

    U64 numberOfDescriptors =
        memoryInfo->memoryMapSize / memoryInfo->descriptorSize;
    result.UEFIPages = CEILING_DIV_VALUE(
        sizeof(PagedMemory) *
            (numberOfDescriptors +
             ADDITIONAL_CAPACITY_FOR_SPLITTING_MEMORY_DESCRIPTOR),
        UEFI_PAGE_SIZE);
    result.memory.len = 0;
    U64 freeMemoryDescriptorsLocation = allocate4KiBPages(result.UEFIPages);
    result.memory.buf = (PagedMemory *)freeMemoryDescriptorsLocation;

    return result;
}

void mapWithSmallestNumberOfPagesInKernelMemory(U64 virt, U64 physical,
                                                U64 bytes) {
    Pages pagesToMap = convertBytesToSmallestNuberOfPages(bytes);
    mapVirtualRegion(virt,
                     (PagedMemory){.start = (U64)physical,
                                   .numberOfPages = pagesToMap.numberOfPages},
                     pagesToMap.pageSize);
}

void identityMapPhysicalMemory(U64 currentHighestAddress) {
    MemoryInfo memoryInfo = getMemoryInfo();
    for (USize i = 0; i < memoryInfo.memoryMapSize / memoryInfo.descriptorSize;
         i++) {
        MemoryDescriptor *desc =
            (MemoryDescriptor *)((U8 *)memoryInfo.memoryMap +
                                 (i * memoryInfo.descriptorSize));
        if (desc->physicalStart + (desc->numberOfPages * UEFI_PAGE_SIZE) >
            currentHighestAddress) {
            currentHighestAddress = desc->physicalStart;
        }
    }

    KFLUSH_AFTER {
        INFO(STRING("highest found address in memory: "));
        INFO((void *)currentHighestAddress, NEWLINE);
    }

    mapWithSmallestNumberOfPagesInKernelMemory(0, 0, currentHighestAddress);
}

KernelMemory convertToKernelMemory(MemoryInfo *memoryInfo,
                                   KernelMemory result) {
    U64 usedPages = VIRTUAL_MEMORY_MAPPER_CAPACITY - bumpFreePages;

    for (U64 i = 0; i < memoryInfo->memoryMapSize / memoryInfo->descriptorSize;
         i++) {
        MemoryDescriptor *desc =
            (MemoryDescriptor *)((U8 *)memoryInfo->memoryMap +
                                 (i * memoryInfo->descriptorSize));

        if (canBeUsedByOS(desc->type)) {
            U64 endAddress =
                desc->physicalStart + desc->numberOfPages * UEFI_PAGE_SIZE;
            if (bumpStartingAddress >= desc->physicalStart &&
                bumpStartingAddress < endAddress) {
                U64 beforeFreePages =
                    (bumpStartingAddress - desc->physicalStart) /
                    UEFI_PAGE_SIZE;
                if (beforeFreePages > 0) {
                    result.memory.buf[result.memory.len] =
                        (PagedMemory){.start = desc->physicalStart,
                                      .numberOfPages = beforeFreePages};
                    result.memory.len++;
                }

                U64 afterStart =
                    bumpStartingAddress + (usedPages * UEFI_PAGE_SIZE);
                U64 afterFreePages = (endAddress - afterStart) / UEFI_PAGE_SIZE;
                if (afterFreePages > 0) {
                    result.memory.buf[result.memory.len] = (PagedMemory){
                        .start = afterStart, .numberOfPages = afterFreePages};
                    result.memory.len++;
                }
            } else {
                result.memory.buf[result.memory.len] =
                    (PagedMemory){.start = desc->physicalStart,
                                  .numberOfPages = desc->numberOfPages};
                result.memory.len++;
            }
        }
    }
    return result;
}
