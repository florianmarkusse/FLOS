#include "efi/memory/physical.h"
#include "abstraction/log.h"
#include "abstraction/memory/virtual/converter.h"
#include "efi-to-kernel/memory/definitions.h"
#include "efi/error.h"
#include "efi/firmware/base.h"
#include "efi/globals.h"
#include "efi/memory/definitions.h"
#include "shared/log.h"
#include "shared/maths/maths.h"
#include "shared/memory/converter.h"
#include "shared/memory/sizes.h"

static MemoryInfo prepareMemoryInfo() {
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

    return memoryInfo;
}

static void fillMemoryInfo(MemoryInfo *memoryInfo) {
    Status status = globals.st->boot_services->get_memory_map(
        &memoryInfo->memoryMapSize, memoryInfo->memoryMap, &memoryInfo->mapKey,
        &memoryInfo->descriptorSize, &memoryInfo->descriptorVersion);
    EXIT_WITH_MESSAGE_IF_EFI_ERROR(status) {
        ERROR(STRING("Getting memory map failed!\n"));
    }
}

static constexpr auto MAX_KERNEL_STRUCTURES = 16;
U64_max_a kernelStructureLocations;

void initKernelStructureLocations(Arena *perm) {
    kernelStructureLocations = (U64_max_a){
        .buf = NEW(perm, U64, MAX_KERNEL_STRUCTURES),
        .len = 0,
        .cap = MAX_KERNEL_STRUCTURES,
    };
}

void addAddressToKernelStructure(U64 address) {
    if (kernelStructureLocations.len >= kernelStructureLocations.cap) {
        EXIT_WITH_MESSAGE {
            ERROR(STRING("Too many kernel structure locations added!\n"));
        }
    }
    kernelStructureLocations.buf[kernelStructureLocations.len] = address;
    kernelStructureLocations.len++;

    KFLUSH_AFTER {
        INFO(STRING("len is now: "));
        INFO(kernelStructureLocations.len, NEWLINE);
    }
}

MemoryInfo getMemoryInfo(Arena *perm) {
    MemoryInfo memoryInfo = prepareMemoryInfo();
    memoryInfo.memoryMap = (MemoryDescriptor *)NEW(
        perm, U8, memoryInfo.memoryMapSize, 0, UEFI_PAGE_SIZE);
    fillMemoryInfo(&memoryInfo);
    return memoryInfo;
}

U64 findHighestMemoryAddress(U64 currentHighestAddress, Arena scratch) {
    MemoryInfo memoryInfo = getMemoryInfo(&scratch);

    FOR_EACH_DESCRIPTOR(&memoryInfo, desc) {
        U64 end = desc->physicalStart + (desc->numberOfPages * UEFI_PAGE_SIZE);
        if (end > currentHighestAddress) {
            currentHighestAddress = end;
        }
    }

    return currentHighestAddress;
}

static bool canBeUsedInEFI(MemoryType type) {
    return type == CONVENTIONAL_MEMORY;
}

typedef struct {
    U64 address;
    U64 alignedAddress;
    U64 padding;
} AlignedMemory;

static void setIfBetterDescriptor(AlignedMemory *current,
                                  AlignedMemory proposed, U64 largerPageSize) {
    if (current->address == U64_MAX) {
        *current = proposed;
        return;
    } else {
        if (!RING_RANGE_VALUE(current->alignedAddress, largerPageSize)) {
            if (RING_RANGE_VALUE(proposed.alignedAddress, largerPageSize)) {
                *current = proposed;
                return;
            }
        }

        if (current->padding > proposed.padding) {
            *current = proposed;
            return;
        }
    }
}

static U64 findAlignedMemory(MemoryInfo *memoryInfo, U64 bytes,
                             U64 minimumAlignment,
                             bool tryEncompassingVirtual) {
    U64 pageSize = pageSizeEncompassing(minimumAlignment);
    if (tryEncompassingVirtual) {
        pageSize = MAX(pageSize, pageSizeEncompassing(bytes));
    }

    U64 largerPageSize;
    if (pageSize == LARGEST_VIRTUAL_PAGE) {
        largerPageSize = LARGEST_VIRTUAL_PAGE;
    } else {
        largerPageSize = increasePageSize(pageSize);
    }

    AlignedMemory bestDescriptor = {
        .address = U64_MAX, .alignedAddress = U64_MAX, .padding = U64_MAX};
    for (; pageSize >= minimumAlignment;
         largerPageSize = pageSize, pageSize = decreasePageSize(pageSize)) {
        FOR_EACH_DESCRIPTOR(memoryInfo, desc) {
            if (!canBeUsedInEFI(desc->type)) {
                continue;
            }

            U64 alignedAddress = ALIGN_UP_VALUE(desc->physicalStart, pageSize);
            U64 originalSize = desc->numberOfPages * UEFI_PAGE_SIZE;
            if (alignedAddress >= desc->physicalStart + originalSize) {
                continue;
            }

            U64 padding = alignedAddress - desc->physicalStart;
            U64 alignedSize = originalSize - padding;
            if (alignedSize < bytes) {
                continue;
            }

            setIfBetterDescriptor(
                &bestDescriptor,
                (AlignedMemory){.address = desc->physicalStart,
                                .alignedAddress = alignedAddress,
                                .padding = padding},
                largerPageSize);
        }

        if (bestDescriptor.address != U64_MAX) {
            Status status = globals.st->boot_services->allocate_pages(
                ALLOCATE_ADDRESS, LOADER_DATA,
                CEILING_DIV_VALUE(bestDescriptor.padding + bytes,
                                  UEFI_PAGE_SIZE),
                &bestDescriptor.address);
            EXIT_WITH_MESSAGE_IF_EFI_ERROR(status) {
                ERROR(STRING("allocating pages for memory failed!\n"));
            }

            if (bestDescriptor.padding) {
                status = globals.st->boot_services->free_pages(
                    bestDescriptor.address,
                    CEILING_DIV_VALUE(bestDescriptor.padding, UEFI_PAGE_SIZE));
                EXIT_WITH_MESSAGE_IF_EFI_ERROR(status) {
                    ERROR(STRING("Freeing padded memory failed!\n"));
                }
            }

            return bestDescriptor.alignedAddress;
        }

        if (pageSize == SMALLEST_VIRTUAL_PAGE) {
            break;
        }
    }

    EXIT_WITH_MESSAGE { ERROR(STRING("Could not find memory!")); }

    __builtin_unreachable();
}

U64 allocateKernelStructure(U64 bytes, U64 minimumAlignment,
                            bool tryEncompassingVirtual, Arena scratch) {
    MemoryInfo memoryInfo = getMemoryInfo(&scratch);

    U64 result = findAlignedMemory(&memoryInfo, bytes, minimumAlignment,
                                   tryEncompassingVirtual);

    addAddressToKernelStructure(result);
    return result;
}

U64 allocateUnalignedMemory(U64 bytes, bool isKernelStructure) {
    U64 address = 0;
    Status status = globals.st->boot_services->allocate_pages(
        ALLOCATE_ANY_PAGES, LOADER_DATA,
        CEILING_DIV_VALUE(bytes, UEFI_PAGE_SIZE), &address);
    EXIT_WITH_MESSAGE_IF_EFI_ERROR(status) {
        ERROR(STRING("allocating unaligned memory failed!\n"));
    }

    if (isKernelStructure) {
        addAddressToKernelStructure(address);
    }
    return address;
}
