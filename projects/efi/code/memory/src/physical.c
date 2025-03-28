#include "efi/memory/physical.h"
#include "abstraction/log.h"
#include "efi-to-kernel/memory/definitions.h"
#include "efi/error.h"
#include "efi/firmware/base.h"
#include "efi/globals.h"
#include "efi/memory/definitions.h"
#include "shared/log.h"
#include "shared/maths/maths.h"
#include "shared/memory/sizes.h"

void fillMemoryInfo(MemoryInfo *memoryInfo) {
    Status status = globals.st->boot_services->get_memory_map(
        &memoryInfo->memoryMapSize, memoryInfo->memoryMap, &memoryInfo->mapKey,
        &memoryInfo->descriptorSize, &memoryInfo->descriptorVersion);
    EXIT_WITH_MESSAGE_IF(status) {
        ERROR(STRING("Getting memory map failed!\n"));
    }
}

U64 findHighestMemoryAddress(U64 currentHighestAddress, Arena scratch) {
    MemoryInfo memoryInfo = prepareMemoryInfo();

    memoryInfo.memoryMap = (MemoryDescriptor *)NEW(
        &scratch, U8, memoryInfo.memoryMapSize, 0, UEFI_PAGE_SIZE);

    fillMemoryInfo(&memoryInfo);

    FOR_EACH_DESCRIPTOR(&memoryInfo, desc) {
        if (desc->physicalStart + (desc->numberOfPages * UEFI_PAGE_SIZE) >
            currentHighestAddress) {
            currentHighestAddress = desc->physicalStart;
        }
    }

    return currentHighestAddress;
}

MemoryInfo prepareMemoryInfo() {
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

static constexpr auto MAX_ALLOWED_ALIGNEMT = 1 * GiB;
static constexpr auto MIN_POSSIBLE_ALIGNMENT = UEFI_PAGE_SIZE;

static void alignmentChecks(U64 alignment) {
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
}

static bool canBeUsedInEFI(MemoryType type) {
    return type == CONVENTIONAL_MEMORY;
}

static U64 findAlignedMemory(MemoryInfo *memoryInfo, U64 bytes,
                             U64 preferredAlignment, U64 minimumAlignment) {
    ASSERT(preferredAlignment >= minimumAlignment);

    for (U64 alignment = MAX(preferredAlignment, MIN_POSSIBLE_ALIGNMENT);
         alignment >= minimumAlignment; alignment /= 2) {
        FOR_EACH_DESCRIPTOR(memoryInfo, desc) {
            if (!canBeUsedInEFI(desc->type)) {
                continue;
            }

            if (RING_RANGE_VALUE(desc->physicalStart, alignment)) {
                continue;
            }

            if (desc->numberOfPages * UEFI_PAGE_SIZE < bytes) {
                continue;
            }

            U64 address = desc->physicalStart;
            Status status = globals.st->boot_services->allocate_pages(
                ALLOCATE_ADDRESS, LOADER_DATA,
                CEILING_DIV_VALUE(bytes, UEFI_PAGE_SIZE), &address);
            EXIT_WITH_MESSAGE_IF(status) {
                ERROR(STRING("allocating pages for memory failed!\n"));
            }

            return address;
        }
    }

    EXIT_WITH_MESSAGE { ERROR(STRING("Could not find memory!")); }

    __builtin_unreachable();
}

U64 getAlignedPhysicalMemoryWithArena(U64 bytes, U64 preferredAlignment,
                                      U64 minimumAlignment, Arena scratch) {
    alignmentChecks(preferredAlignment);
    alignmentChecks(minimumAlignment);
    MemoryInfo memoryInfo = prepareMemoryInfo();

    memoryInfo.memoryMap = (MemoryDescriptor *)NEW(
        &scratch, U8, memoryInfo.memoryMapSize, 0, UEFI_PAGE_SIZE);

    fillMemoryInfo(&memoryInfo);

    return findAlignedMemory(&memoryInfo, bytes, preferredAlignment,
                             minimumAlignment);
}

U64 getAlignedPhysicalMemory(U64 bytes, U64 preferredAlignment,
                             U64 minimumAlignment) {
    alignmentChecks(preferredAlignment);
    alignmentChecks(minimumAlignment);
    MemoryInfo memoryInfo = prepareMemoryInfo();

    // NOTE: Adding an extra page here to acoount for allocation below
    memoryInfo.memoryMapSize += UEFI_PAGE_SIZE;
    Status status = globals.st->boot_services->allocate_pages(
        ALLOCATE_ANY_PAGES, LOADER_DATA,
        CEILING_DIV_VALUE(memoryInfo.memoryMapSize, UEFI_PAGE_SIZE),
        (PhysicalAddress *)&memoryInfo.memoryMap);
    EXIT_WITH_MESSAGE_IF(status) {
        ERROR(STRING("allocating pages for getting memory map failed!\n"));
    }

    fillMemoryInfo(&memoryInfo);
    U64 result = findAlignedMemory(&memoryInfo, bytes, preferredAlignment,
                                   minimumAlignment);

    status = globals.st->boot_services->free_pages(
        (U64)memoryInfo.memoryMap,
        CEILING_DIV_VALUE(memoryInfo.memoryMapSize, UEFI_PAGE_SIZE));
    EXIT_WITH_MESSAGE_IF(status) {
        ERROR(STRING("Freeing allocated memory for memory map failed!\n"));
    }

    return result;
}
