#include "shared/memory/virtual.h"

#include "abstraction/memory/virtual/map.h"
#include "efi-to-kernel/memory/definitions.h"
#include "shared/assert.h"
#include "shared/maths/maths.h"
#include "x86/memory/definitions.h"

Range_max_a freeVirtualMemory;

void initVirtualMemoryManager(VirtualMemory virt) {
    freeVirtualMemory = virt.freeVirtualMemory;
}

U64 getVirtualMemory(U64 size, U64 align) {
    //    U64 alignedUpValue = ALIGN_UP_VALUE(higherHalfRegion.start, align);
    //
    //    ASSERT(higherHalfRegion.start <= alignedUpValue);
    //    ASSERT(alignedUpValue <= higherHalfRegion.end);
    //    ASSERT(higherHalfRegion.end - (alignedUpValue + size) <
    //           higherHalfRegion.start);
    //
    //    higherHalfRegion.start = alignedUpValue;
    //    U64 result = higherHalfRegion.start;
    //
    //    higherHalfRegion.start += size;
    return 0;
}
