#include "os-loader/memory.h"

#include "abstraction/log.h"
#include "abstraction/memory/manipulation.h"
#include "abstraction/memory/physical/allocation.h"
#include "abstraction/memory/virtual/converter.h"
#include "abstraction/memory/virtual/map.h"
#include "efi/error.h"
#include "efi/firmware/system.h"
#include "efi/globals.h"
#include "efi/memory/physical.h"
#include "shared/log.h"
#include "shared/maths/maths.h"
#include "shared/memory/converter.h"
#include "shared/text/string.h"
#include "shared/trees/red-black.h"

static bool memoryTypeCanBeUsedByKernel(MemoryType type) {
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

void allocateSpaceForKernelMemory(Arena scratch, KernelMemory *location) {
    MemoryInfo memoryInfo = getMemoryInfo(&scratch);
    U64 numberOfDescriptors =
        memoryInfo.memoryMapSize / memoryInfo.descriptorSize;
    U64 totalNumberOfDescriptors = ((numberOfDescriptors * 3) / 2);
    U64 bytes = sizeof(RedBlackNode) * totalNumberOfDescriptors;

    U8 *freeMemoryDescriptorsLocation =
        (U8 *)allocateKernelStructure(bytes, 0, false, scratch);
    if (!freeMemoryDescriptorsLocation) {
        EXIT_WITH_MESSAGE {
            ERROR(STRING("Received the 0 memory address to use for the memory "
                         "descriptors allocator!\n"));
        }
    }
    Arena physicalMemoryArena =
        (Arena){.curFree = freeMemoryDescriptorsLocation,
                .beg = freeMemoryDescriptorsLocation,
                .end = freeMemoryDescriptorsLocation + bytes};

    RedBlackNode *tree = NEW(&physicalMemoryArena, RedBlackNode);
    tree = nullptr;

    *location = (KernelMemory){.allocator = physicalMemoryArena, .tree = tree};
}

U64 alignVirtual(U64 virt, U64 physical, U64 bytes) {
    U64 alignment = pageSizeEncompassing(bytes * 2);

    virt = ALIGN_UP_VALUE(virt, alignment);
    virt = virt | RING_RANGE_VALUE(physical, alignment);

    return virt;
}

U64 mapMemoryWithFlags(U64 virt, U64 physical, U64 bytes, U64 flags) {
    KFLUSH_AFTER {
        INFO(STRING("virt: "));
        INFO((void *)virt);
        INFO(STRING(" physical: "));
        INFO((void *)physical);
        INFO(STRING(" bytes: "));
        INFO(bytes);
        INFO(STRING(" flags: "));
        INFO(flags, NEWLINE);
    }

    for (U64 bytesMapped = 0, mappingSize; bytesMapped < bytes;
         virt += mappingSize, physical += mappingSize,
             bytesMapped += mappingSize) {
        mappingSize = pageSizeLeastLargerThan(physical, bytes - bytesMapped);

        KFLUSH_AFTER {
            INFO(STRING("I found a mapping size of: "));
            INFO(mappingSize);
            INFO(STRING(":mapping  "));
            INFO((void *)virt);
            INFO(STRING("to "));
            INFO((void *)physical, NEWLINE);
        }

        mapPageWithFlags(virt, physical, mappingSize, flags);
    }
    return virt;
}

U64 mapMemory(U64 virt, U64 physical, U64 bytes) {
    return mapMemoryWithFlags(virt, physical, bytes,
                              KERNEL_STANDARD_PAGE_FLAGS);
}

static bool isKernelStructure(U64 address) {
    for (U64 i = 0; i < kernelStructureLocations.len; i++) {
        if (kernelStructureLocations.buf[i] == address) {
            return true;
        }
    }
    return false;
}

void convertToKernelMemory(MemoryInfo *memoryInfo, KernelMemory *location) {
    FOR_EACH_DESCRIPTOR(memoryInfo, desc) {
        if (memoryTypeCanBeUsedByKernel(desc->type) &&
            !isKernelStructure(desc->physicalStart)) {
            if (desc->physicalStart == 0) {
                if (desc->numberOfPages == 1) {
                    continue;
                }

                desc->physicalStart += UEFI_PAGE_SIZE;
                desc->physicalStart--;
            }

            RedBlackNode *node = NEW(&location->allocator, RedBlackNode);
            node->bytes = desc->numberOfPages * UEFI_PAGE_SIZE;
            node->start = desc->physicalStart;
            insertRedBlackNode(&location->tree, node);
        }
    }
}
