#include "os-loader/memory.h"

#include "abstraction/log.h"
#include "abstraction/memory/manipulation.h"
#include "abstraction/memory/virtual/converter.h"
#include "abstraction/memory/virtual/map.h"
#include "abstraction/thread.h"
#include "efi-to-kernel/memory/definitions.h"
#include "efi/error.h"
#include "efi/firmware/system.h"
#include "efi/globals.h"
#include "efi/memory/physical.h"
#include "efi/memory/virtual.h"
#include "shared/log.h"
#include "shared/maths.h"
#include "shared/memory/converter.h"
#include "shared/memory/management/management.h"
#include "shared/memory/management/status.h"
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

static constexpr auto RED_COLOR = 0xFF0000;

void kernelPhysicalBuddyPrepare(BuddyWithNodeAllocator *kernelBuddyPhysical,
                                GraphicsOutputProtocolMode *mode,
                                Arena scratch) {
    MemoryInfo memoryInfo = getMemoryInfo(&scratch);
    U32 numberOfDescriptors =
        (U32)(memoryInfo.memoryMapSize / memoryInfo.descriptorSize);
    // NOTE: just to hold the initial descriptors before moved into final kernel
    // state.
    U32 expectedNumberOfDescriptors = numberOfDescriptors * 8;

    nodeAllocatorInit(
        &kernelBuddyPhysical->nodeAllocator,
        (void_a){.buf =
                     NEW(&globals.kernelTemporary,
                         typeof(*kernelBuddyPhysical->buddy.data.blocksFree[0]),
                         .count = expectedNumberOfDescriptors),
                 .len = expectedNumberOfDescriptors *
                        sizeof(*kernelBuddyPhysical->buddy.data.blocksFree[0])},
        (void_a){.buf = NEW(&globals.kernelTemporary, void *,
                            .count = expectedNumberOfDescriptors),
                 .len = expectedNumberOfDescriptors * sizeof(void *)},
        sizeof(*kernelBuddyPhysical->buddy.data.blocksFree[0]),
        alignof(*kernelBuddyPhysical->buddy.data.blocksFree[0]));

    buddyInit(&kernelBuddyPhysical->buddy, 40);

    if (setjmp(kernelBuddyPhysical->buddy.jmpBuf)) {
        drawStatusRectangle(mode, RED_COLOR);
        hangThread();
    }
}

void convertToKernelMemory(MemoryInfo *memoryInfo,
                           BuddyWithNodeAllocator *kernelBuddyPhysical) {
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
                for (typeof(kernelStructureLocations.len) i = 0;
                     i < kernelStructureLocations.len; i++) {
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

            for (typeof(availableMemory.len) i = 0; i < availableMemory.len;
                 i++) {
                U64 addressEnd =
                    availableMemory.buf[i].start + availableMemory.buf[i].bytes;
                buddyFreeRegionAdd(&kernelBuddyPhysical->buddy,
                                   availableMemory.buf[i].start, addressEnd,
                                   &kernelBuddyPhysical->nodeAllocator);
            }
        }
    }
}
