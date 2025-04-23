#include "shared/memory/virtual.h"

#include "abstraction/memory/virtual/map.h"
#include "efi-to-kernel/memory/definitions.h"
#include "shared/assert.h"
#include "shared/maths/maths.h"

// Start is set in the init function.
Memory higherHalfRegion = {.bytes = KERNEL_CODE_START};
Memory lowerHalfRegion = {.bytes = LOWER_HALF_END};

void initVirtualMemoryManager(VirtualMemory virt) {
    lowerHalfRegion.start = virt.availableLowerHalfAddress;
    lowerHalfRegion.bytes = LOWER_HALF_END - lowerHalfRegion.start;
    higherHalfRegion.start = virt.availableHigherHalfAddress;
    higherHalfRegion.bytes = KERNEL_CODE_START - higherHalfRegion.start;
}

U64 getVirtualMemory(U64 size, U64 align) {
    U64 alignedUpValue = ALIGN_UP_VALUE(higherHalfRegion.start, align);

    ASSERT(higherHalfRegion.start <= alignedUpValue);
    ASSERT(alignedUpValue <= higherHalfRegion.end);
    ASSERT(higherHalfRegion.end - (alignedUpValue + size) <
           higherHalfRegion.start);

    higherHalfRegion.start = alignedUpValue;
    U64 result = higherHalfRegion.start;

    higherHalfRegion.start += size;
    return result;
}
