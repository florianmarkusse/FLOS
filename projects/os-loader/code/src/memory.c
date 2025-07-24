#include "os-loader/memory.h"

#include "abstraction/log.h"
#include "abstraction/memory/manipulation.h"
#include "abstraction/memory/virtual/converter.h"
#include "abstraction/memory/virtual/map.h"
#include "abstraction/thread.h"
#include "efi/error.h"
#include "efi/firmware/system.h"
#include "efi/globals.h"
#include "efi/memory/physical.h"
#include "shared/log.h"
#include "shared/maths.h"
#include "shared/memory/converter.h"
#include "shared/memory/management/management.h"
#include "shared/text/string.h"
#include "shared/trees/red-black/memory-manager.h"

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

void allocateSpaceForKernelMemory(
    RedBlackMMTreeWithFreeList *redBlackMMTreeWithFreeList, Arena scratch) {
    MemoryInfo memoryInfo = getMemoryInfo(&scratch);
    U64 numberOfDescriptors =
        memoryInfo.memoryMapSize / memoryInfo.descriptorSize;
    // NOTE: just to hold the initial descriptors before moved into final kernel
    // state.
    U64 expectedNumberOfDescriptors = numberOfDescriptors * 2;

    createDynamicArray(expectedNumberOfDescriptors,
                       sizeof(*redBlackMMTreeWithFreeList->nodes.buf),
                       alignof(*redBlackMMTreeWithFreeList->nodes.buf),
                       (void_max_a *)&redBlackMMTreeWithFreeList->nodes,
                       scratch);
    createDynamicArray(expectedNumberOfDescriptors,
                       sizeof(*redBlackMMTreeWithFreeList->freeList.buf),
                       alignof(*redBlackMMTreeWithFreeList->freeList.buf),
                       (void_max_a *)&redBlackMMTreeWithFreeList->freeList,
                       scratch);
}

U64 alignVirtual(U64 virtualAddress, U64 physicalAddress, U64 bytes) {
    U64 alignment = pageSizeEncompassing(bytes);

    U64 result = ALIGN_UP_VALUE(virtualAddress, alignment);
    result |= RING_RANGE_VALUE(physicalAddress, alignment);

    return result;
}

U64 mapMemory(U64 virt, U64 physical, U64 bytes, U64 flags) {
    for (U64 bytesMapped = 0, mappingSize; bytesMapped < bytes;
         virt += mappingSize, physical += mappingSize,
             bytesMapped += mappingSize) {
        mappingSize = pageSizeLeastLargerThan(physical, bytes - bytesMapped);

        mapPageWithFlags(virt, physical, mappingSize, flags);
    }
    return virt;
}

static constexpr auto RED_COLOR = 0xFF0000;

void convertToKernelMemory(
    MemoryInfo *memoryInfo,
    PackedMMTreeWithFreeList *physicalMemoryTree,
    RedBlackMMTreeWithFreeList *RedBlackMMtreeWithFreeList,
    GraphicsOutputProtocolMode *mode) {
    FOR_EACH_DESCRIPTOR(memoryInfo, desc) {
        if (memoryTypeCanBeUsedByKernel(desc->type)) {
            if (desc->physicalStart == 0) {
                if (desc->numberOfPages == 1) {
                    continue;
                }

                // Keep the first 4096 bytes unused.
                desc->physicalStart += UEFI_PAGE_SIZE;
                desc->numberOfPages--;
            }

            Memory availableMemoryBuf[MAX_KERNEL_STRUCTURES];
            Memory_a availableMemory = {
                .buf = availableMemoryBuf,
                .len = 0,
            };

            U64 curStart = desc->physicalStart;
            U64 curEnd = curStart + desc->numberOfPages * UEFI_PAGE_SIZE;
            U64 kernelSize = 0;

            U64 descriptorEnd = curEnd;
            while (curStart < descriptorEnd) {
                for (U64 i = 0; i < kernelStructureLocations.len; i++) {
                    U64 kernelStart = kernelStructureLocations.buf[i].start;

                    if (kernelStart >= curStart && kernelStart < curEnd) {
                        curEnd = kernelStart;
                        kernelSize = kernelStructureLocations.buf[i].bytes;
                    }
                }

                U64 availableBytes = curEnd - curStart;
                if (availableBytes) {
                    availableMemory.buf[availableMemory.len] =
                        (Memory){.start = curStart, .bytes = availableBytes};
                    availableMemory.len++;
                }

                curStart = curEnd + kernelSize;
                curEnd = descriptorEnd;
            }

            for (U64 i = 0; i < availableMemory.len; i++) {
                MMNode *node =
                    getMMNode(&RedBlackMMtreeWithFreeList->freeList,
                                      &RedBlackMMtreeWithFreeList->nodes);
                if (!node) {
                    drawStatusRectangle(mode, RED_COLOR);
                    hangThread();
                }
                node->memory = availableMemory.buf[i];
                insertMMNodeAndAddToFreelist(
                    &RedBlackMMtreeWithFreeList->tree, node,
                    &RedBlackMMtreeWithFreeList->freeList);
            }
        }
    }

    setPackedMemoryAllocator((PackedTreeWithFreeList *)physicalMemoryTree,
                             (TreeWithFreeList *)RedBlackMMtreeWithFreeList);
}
