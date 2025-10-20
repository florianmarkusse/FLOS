#include "abstraction/memory/virtual/allocator.h"

#include "abstraction/log.h"
#include "abstraction/memory/manipulation.h"
#include "abstraction/memory/virtual/map.h"

#include "efi-to-kernel/memory/definitions.h"
#include "efi/error.h"
#include "efi/globals.h"
#include "efi/memory/physical.h"
#include "efi/memory/virtual.h"
#include "shared/log.h"
#include "shared/maths.h"
#include "shared/memory/allocator/macros.h"
#include "shared/memory/converter.h"
#include "shared/memory/management/page.h"
#include "shared/memory/management/status.h"
#include "shared/types/numeric.h"

void *memoryZeroedForVirtualGet(VirtualAllocationType type) {
    void *result = findAlignedMemoryBlock(
        virtualStructBytes[type], UEFI_PAGE_SIZE, globals.uefiMemory, false);
    memset(result, 0, virtualStructBytes[type]);
    return result;
}

void memoryZeroedForVirtualFree(U64 address, VirtualAllocationType type) {
    (void)address;
    (void)type;
    EXIT_WITH_MESSAGE { ERROR(STRING("Not required for EFI implementation!")); }
}

U64 alignVirtual(U64 virtualAddress, U64 physicalAddress, U64 bytes) {
    U64_pow2 alignment = pageSizeEncompassing(bytes);

    U64 result = alignUp(virtualAddress, alignment);
    // NOTE: if the physical address is somehow not as aligned as the virtual
    // one. So, we can map 1 4KiB page and then the remaining as 2MiB page, for
    // example, instead of the virtual and physical address being out of
    // alignment and only being able to do 4KiB mappings.
    result |= ringBufferIndex(physicalAddress, alignment);

    return result;
}

U64 mapMemory(U64 virt, U64 physical, U64 bytes, U64 flags) {
    U64_pow2 mappingSize;
    for (typeof(bytes) bytesMapped = 0; bytesMapped < bytes;
         virt += mappingSize, physical += mappingSize,
                       bytesMapped += mappingSize) {
        mappingSize = pageSizeLeastLargerThan(physical, bytes - bytesMapped);
        mapPage(virt, physical, mappingSize, .flags = flags);
    }
    return virt;
}

StackResult stackCreateAndMap(U64 virtualMemoryFirstAvailable, U64 stackSize,
                              bool attemptLargestMapping) {
    U64 stackAddress =
        (U64)findAlignedMemoryBlock(stackSize, KERNEL_STACK_ALIGNMENT,
                                    globals.uefiMemory, attemptLargestMapping);

    // NOTE: Overflow precaution
    virtualMemoryFirstAvailable += stackSize;
    virtualMemoryFirstAvailable =
        alignVirtual(virtualMemoryFirstAvailable, stackAddress, stackSize);
    U64 stackGuardPageAddress = virtualMemoryFirstAvailable - stackSize;
    addPageMapping((Memory){.start = stackGuardPageAddress, .bytes = stackSize},
                   GUARD_PAGE_SIZE);

    KFLUSH_AFTER {
        mappingVirtualGuardPageAppend(stackGuardPageAddress, KERNEL_STACK_SIZE);
    }

    U64 stackVirtualStart = virtualMemoryFirstAvailable;
    virtualMemoryFirstAvailable =
        mapMemory(virtualMemoryFirstAvailable, stackAddress, stackSize,
                  pageFlagsReadWrite() | pageFlagsNoCacheEvict());

    KFLUSH_AFTER {
        mappingMemoryAppend(stackVirtualStart, stackAddress, stackSize);
    }

    return (StackResult){.stackVirtualTop = stackVirtualStart + stackSize,
                         .virtualMemoryFirstAvailable =
                             virtualMemoryFirstAvailable};
}
