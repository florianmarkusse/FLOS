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

PhysicalAddress allocAndZero(USize numPages) {
    PhysicalAddress page = allocate4KiBPages(numPages);
    /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
    memset((void *)page, 0, numPages * UEFI_PAGE_SIZE);
    return page;
}

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

    U64 largestPageSize = pageSizes[MEMORY_PAGE_SIZES_COUNT - 1];
    U64 numberOfRequiredPages =
        CEILING_DIV_VALUE(currentHighestAddress, largestPageSize);
    mapVirtualRegion(
        0, (PagedMemory){.start = 0, .numberOfPages = numberOfRequiredPages},
        largestPageSize);
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

KernelMemory convertToKernelMemory(MemoryInfo *memoryInfo,
                                   KernelMemory result) {
    U64 usedPages = BUMP_ALLOCATOR_PAGE_INITIAL_CAPACITY - bumpFreePages;

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
