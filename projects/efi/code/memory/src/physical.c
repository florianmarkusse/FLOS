#include "efi/memory/physical.h"
#include "abstraction/log.h"
#include "abstraction/memory/virtual/converter.h"
#include "efi-to-kernel/memory/definitions.h"
#include "efi/error.h"
#include "efi/firmware/base.h"
#include "efi/globals.h"
#include "shared/log.h"
#include "shared/maths.h"
#include "shared/memory/converter.h"
#include "shared/memory/management/definitions.h"
#include "shared/memory/sizes.h"

bool memoryTypeCanBeUsedByKernel(MemoryType type) {
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

static bool needsToBeMappedByKernel(MemoryType type) {
    return memoryTypeCanBeUsedByKernel(type) || type == RUNTIME_SERVICES_CODE ||
           type == RUNTIME_SERVICES_DATA || type == ACPI_RECLAIM_MEMORY ||
           type == ACPI_MEMORY_NVS || type == MEMORY_MAPPED_IO ||
           type == MEMORY_MAPPED_IO_PORT_SPACE || type == PAL_CODE;
}

static bool canBeUsedInEFI(MemoryType type) {
    return type == CONVENTIONAL_MEMORY;
}

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

__attribute__((malloc, aligned(UEFI_PAGE_SIZE))) static void *
pagesAllocAll(AllocateType allocateType, U64 bytes, U64 *address) {
    Status status = globals.st->boot_services->allocate_pages(
        allocateType, LOADER_DATA, ceilingDivide(bytes, UEFI_PAGE_SIZE),
        address);

    if (!(*address)) {
        EXIT_WITH_MESSAGE {
            ERROR(STRING("Received the 0 memory address to use for the memory "
                         "tree allocator!\n"));
        }
    }

    EXIT_WITH_MESSAGE_IF_EFI_ERROR(status) {
        ERROR(STRING("allocating pages for memory failed!\n"));
    }

    return (void *)*address;
}

void *pagesAlloc(U64 bytes) {
    U64 address = 0;
    return pagesAllocAll(ALLOCATE_ANY_PAGES, bytes, &address);
}

static void fillMemoryInfo(MemoryInfo *memoryInfo) {
    Status status = globals.st->boot_services->get_memory_map(
        &memoryInfo->memoryMapSize, memoryInfo->memoryMap, &memoryInfo->mapKey,
        &memoryInfo->descriptorSize, &memoryInfo->descriptorVersion);
    EXIT_WITH_MESSAGE_IF_EFI_ERROR(status) {
        ERROR(STRING("Getting memory map failed!\n"));
    }
}

MemoryInfo memoryInfoGet(Arena *perm) {
    MemoryInfo memoryInfo = prepareMemoryInfo();
    memoryInfo.memoryMap = (MemoryDescriptor *)NEW(
        perm, U8, .count = memoryInfo.memoryMapSize, .align = UEFI_PAGE_SIZE);
    fillMemoryInfo(&memoryInfo);
    return memoryInfo;
}

U64 highestMemoryAddressFind(U64 currentHighestAddress, Arena scratch) {
    MemoryInfo memoryInfo = memoryInfoGet(&scratch);

    FOR_EACH_DESCRIPTOR(&memoryInfo, desc) {
        if (needsToBeMappedByKernel(desc->type)) {
            U64 end =
                desc->physicalStart + (desc->numberOfPages * UEFI_PAGE_SIZE);
            if (end > currentHighestAddress) {
                currentHighestAddress = end;
            }
        }
    }

    return currentHighestAddress;
}

typedef struct {
    U64 address;
    U64 alignedAddress;
    U64 padding;
} AlignedMemory;

static void setIfBetterDescriptor(AlignedMemory *current,
                                  AlignedMemory proposed,
                                  U64_pow2 largerAlignment) {
    if (current->address == U64_MAX) {
        *current = proposed;
        return;
    } else {
        if (!ringBufferIndex(current->alignedAddress, largerAlignment)) {
            if (ringBufferIndex(proposed.alignedAddress, largerAlignment)) {
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

KernelStructures kernelStructureLocations;

static void addAddressToKernelStructure(U64 address, U64 bytes) {
    if (kernelStructureLocations.len >= MAX_KERNEL_STRUCTURES) {
        EXIT_WITH_MESSAGE {
            ERROR(STRING("Too many kernel structure locations added!\n"));
        }
    }

    kernelStructureLocations.buf[kernelStructureLocations.len] =
        (Memory){.start = address, .bytes = bytes};
    kernelStructureLocations.len++;
}

void *alignedMemoryBlockAlloc(U64_pow2 bytes, U64_pow2 alignment, Arena scratch,
                             bool attemptLargestMapping) {
    MemoryInfo memoryInfo = memoryInfoGet(&scratch);

    U64_pow2 pageSize = pageSizeEncompassing(alignment);
    if (attemptLargestMapping) {
        pageSize = MAX(pageSize, pageSizeEncompassing(bytes));
    }

    U64_pow2 largerPageSize;
    if (pageSize == pageSizeLargest()) {
        largerPageSize = pageSize;
    } else {
        largerPageSize = increasePageSize(pageSize);
    }

    AlignedMemory bestDescriptor = {
        .address = U64_MAX, .alignedAddress = U64_MAX, .padding = U64_MAX};
    while (pageSize >= alignment && bestDescriptor.address == U64_MAX) {
        FOR_EACH_DESCRIPTOR(&memoryInfo, desc) {
            if (!canBeUsedInEFI(desc->type)) {
                continue;
            }

            U64 alignedAddress = alignUp(desc->physicalStart, pageSize);
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

        largerPageSize = pageSize;
        if (pageSize > pageSizeSmallest()) {
            pageSize = decreasePageSize(pageSize);
        } else {
            pageSize = 0;
        }
    }

    if (bestDescriptor.address != MAX_VALUE(bestDescriptor.address)) {
        pagesAllocAll(ALLOCATE_ADDRESS, bestDescriptor.padding + bytes,
                         &bestDescriptor.address);

        if (bestDescriptor.padding) {
            Status status = globals.st->boot_services->free_pages(
                bestDescriptor.address,
                ceilingDivide(bestDescriptor.padding, UEFI_PAGE_SIZE));
            EXIT_WITH_MESSAGE_IF_EFI_ERROR(status) {
                ERROR(STRING("Freeing padded memory failed!\n"));
            }
        }

        addAddressToKernelStructure(bestDescriptor.alignedAddress, bytes);

        return (void *)bestDescriptor.alignedAddress;
    }

    EXIT_WITH_MESSAGE { ERROR(STRING("Could not find memory!")); }

    __builtin_unreachable();
}
