#include "os-loader/memory.h"

#include "abstraction/log.h"
#include "abstraction/memory/manipulation.h"
#include "abstraction/memory/physical.h"
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

Arena allocateSpaceForKernelMemory(Arena scratch) {
    MemoryInfo memoryInfo = getMemoryInfo(&scratch);
    U64 numberOfDescriptors =
        memoryInfo.memoryMapSize / memoryInfo.descriptorSize;
    U64 totalNumberOfDescriptors = ((numberOfDescriptors * 3) / 2);

    return createAllocatorForMemoryTree(totalNumberOfDescriptors, scratch);
}

U64 alignVirtual(U64 virt, U64 physical, U64 bytes) {
    U64 alignment = pageSizeEncompassing(bytes * 2);

    virt = ALIGN_UP_VALUE(virt, alignment);
    virt = virt | RING_RANGE_VALUE(physical, alignment);

    return virt;
}

U64 mapMemoryWithFlags(U64 virt, U64 physical, U64 bytes, U64 flags) {
    for (U64 bytesMapped = 0, mappingSize; bytesMapped < bytes;
         virt += mappingSize, physical += mappingSize,
             bytesMapped += mappingSize) {
        mappingSize = pageSizeLeastLargerThan(physical, bytes - bytesMapped);

        mapPageWithFlags(virt, physical, mappingSize, flags);
    }
    return virt;
}

U64 mapMemory(U64 virt, U64 physical, U64 bytes) {
    return mapMemoryWithFlags(virt, physical, bytes,
                              KERNEL_STANDARD_PAGE_FLAGS);
}

void convertToKernelMemory(MemoryInfo *memoryInfo,
                           PackedMemoryTree *physicalMemoryTree,
                           Arena treeAllocator) {
    RedBlackNodeMM *root = nullptr;

    FOR_EACH_DESCRIPTOR(memoryInfo, desc) {
        if (memoryTypeCanBeUsedByKernel(desc->type)) {
            if (desc->physicalStart == 0) {
                if (desc->numberOfPages == 1) {
                    continue;
                }

                desc->physicalStart += UEFI_PAGE_SIZE;
                desc->physicalStart--;
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
                RedBlackNodeMM *node = NEW(&treeAllocator, RedBlackNodeMM);
                node->memory = availableMemory.buf[i];
                // TODO: CAN FREE THE STUFF HERE I THINK!!!
                insertRedBlackNodeMM(&root, node);
            }
        }
    }

    *physicalMemoryTree = (PackedMemoryTree){
        .allocator = (PackedArena){.beg = treeAllocator.beg,
                                   .curFree = treeAllocator.curFree,
                                   .end = treeAllocator.end},
        .tree = root};
}
