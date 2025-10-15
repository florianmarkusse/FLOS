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

void kernelPhysicalBuddyPrepare(Buddy *kernelBuddyPhysical,
                                U64 physicalAddressMaxExclusive,
                                GraphicsOutputProtocolMode *mode) {
    U32 slotsRequired =
        MAX((U32)ceilingDivide(physicalAddressMaxExclusive,
                               1 << BUDDY_PHYSICAL_PAGE_SIZE_MAX) *
                2,
            BUDDY_BLOCKS_CAPACITY_PER_ORDER_DEFAULT);

    Exponent orderCount =
        buddyOrderCountOnLargestPageSize(BUDDY_PHYSICAL_PAGE_SIZE_MAX);
    U64 *backingBuffer =
        NEW(&globals.kernelPermanent, U64, .count = orderCount * slotsRequired);
    buddyInit(kernelBuddyPhysical, backingBuffer, slotsRequired, orderCount);

    if (setjmp(kernelBuddyPhysical->memoryExhausted)) {
        drawStatusRectangle(mode, RED_COLOR);
        hangThread();
    }
    if (setjmp(kernelBuddyPhysical->backingBufferExhausted)) {
        drawStatusRectangle(mode, RED_COLOR);
        hangThread();
    }
}

void convertToKernelMemory(MemoryInfo *memoryInfo, Buddy *kernelBuddyPhysical,
                           U64 *physicalMemoryTotal) {
    U64 physicalMemoryBytes = 0;
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
                physicalMemoryBytes += availableMemory.buf[i].bytes;
                buddyFree(kernelBuddyPhysical, availableMemory.buf[i]);
            }
        }
    }

    *physicalMemoryTotal = physicalMemoryBytes;
}
