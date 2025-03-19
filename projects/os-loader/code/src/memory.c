#include "os-loader/memory.h"

#include "abstraction/log.h"
#include "abstraction/memory/manipulation.h"
#include "abstraction/memory/physical/allocation.h"
#include "abstraction/memory/virtual/converter.h"
#include "abstraction/memory/virtual/map.h"
#include "efi/error.h"
#include "efi/firmware/system.h"
#include "efi/globals.h"
#include "efi/memory/definitions.h"
#include "efi/memory/physical.h"
#include "shared/log.h"
#include "shared/maths/maths.h"
#include "shared/memory/converter.h"
#include "shared/text/string.h"

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

static constexpr auto ADDITIONAL_CAPACITY_FOR_SPLITTING_MEMORY_DESCRIPTOR = 1;
KernelMemory stubMemoryBeforeExitBootServices(MemoryInfo *memoryInfo) {
    /*KernelMemory result;*/
    /**/
    /*U64 numberOfDescriptors =*/
    /*    memoryInfo->memoryMapSize / memoryInfo->descriptorSize;*/
    /*result.UEFIPages = CEILING_DIV_VALUE(*/
    /*    sizeof(PagedMemory) **/
    /*        (numberOfDescriptors +*/
    /*         ADDITIONAL_CAPACITY_FOR_SPLITTING_MEMORY_DESCRIPTOR),*/
    /*    UEFI_PAGE_SIZE);*/
    /*result.memory.len = 0;*/
    /*U64 freeMemoryDescriptorsLocation = allocate4KiBPages(result.UEFIPages);*/
    /*result.memory.buf = (PagedMemory *)freeMemoryDescriptorsLocation;*/
    /**/
    /*return result;*/
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
    /*MemoryInfo memoryInfo = getMemoryInfo();*/
    /*FOR_EACH_DESCRIPTOR(&memoryInfo, desc) {*/
    /*    if (desc->physicalStart + (desc->numberOfPages * UEFI_PAGE_SIZE) >*/
    /*        currentHighestAddress) {*/
    /*        currentHighestAddress = desc->physicalStart;*/
    /*    }*/
    /*}*/
    /**/
    /*KFLUSH_AFTER {*/
    /*    INFO(STRING("highest found address in memory: "));*/
    /*    INFO((void *)currentHighestAddress, NEWLINE);*/
    /*}*/
    /**/
    /*mapWithSmallestNumberOfPagesInKernelMemory(0, 0, currentHighestAddress);*/
}

KernelMemory convertToKernelMemory(MemoryInfo *memoryInfo,
                                   KernelMemory result) {
    /*U64 usedPages = VIRTUAL_MEMORY_MAPPER_CAPACITY - virtualMemoryMapperEmd;*/
    /**/
    /*FOR_EACH_DESCRIPTOR(memoryInfo, desc) {*/
    /*    if (canBeUsedByOS(desc->type)) {*/
    /*        U64 endAddress =*/
    /*            desc->physicalStart + desc->numberOfPages * UEFI_PAGE_SIZE;*/
    /*        if (virtualMemoryMapperFree >= desc->physicalStart &&*/
    /*            virtualMemoryMapperFree < endAddress) {*/
    /*            U64 beforeFreePages =*/
    /*                (virtualMemoryMapperFree - desc->physicalStart) /*/
    /*                UEFI_PAGE_SIZE;*/
    /*            if (beforeFreePages > 0) {*/
    /*                result.memory.buf[result.memory.len] =*/
    /*                    (PagedMemory){.start = desc->physicalStart,*/
    /*                                  .numberOfPages = beforeFreePages};*/
    /*                result.memory.len++;*/
    /*            }*/
    /**/
    /*            U64 afterStart =*/
    /*                virtualMemoryMapperFree + (usedPages * UEFI_PAGE_SIZE);*/
    /*            U64 afterFreePages = (endAddress - afterStart) /
     * UEFI_PAGE_SIZE;*/
    /*            if (afterFreePages > 0) {*/
    /*                result.memory.buf[result.memory.len] = (PagedMemory){*/
    /*                    .start = afterStart, .numberOfPages =
     * afterFreePages};*/
    /*                result.memory.len++;*/
    /*            }*/
    /*        } else {*/
    /*            result.memory.buf[result.memory.len] =*/
    /*                (PagedMemory){.start = desc->physicalStart,*/
    /*                              .numberOfPages = desc->numberOfPages};*/
    /*            result.memory.len++;*/
    /*        }*/
    /*    }*/
    /*}*/
    /*return result;*/
}
