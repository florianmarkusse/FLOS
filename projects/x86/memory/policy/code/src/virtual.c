#include "x86/memory/policy/virtual.h"
#include "abstraction/memory/management/policy.h"

#include "efi-to-kernel/memory/definitions.h"
#include "shared/assert.h"
#include "shared/maths/maths.h"
#include "shared/types/types.h"
#include "x86/configuration/cpu.h"
#include "x86/memory/pat.h"
#include "x86/memory/virtual.h"

// Start is set in the init function.
VirtualRegion higherHalfRegion = {.end = KERNEL_CODE_START};
VirtualRegion lowerHalfRegion = {.end = LOWER_HALF_END};

void initVirtualMemoryManager(VirtualMemory virt) {
    lowerHalfRegion.start = virt.availableLowerHalfAddress;
    higherHalfRegion.start = virt.availableHigherHalfAddress;

    /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
    rootPageTable = (VirtualPageTable *)CR3();
}

U64 getVirtualMemory(U64 size, PageSize alignValue) {
    ASSERT(size <= X86_512GIB_PAGE);
    U64 alignedUpValue = ALIGN_UP_VALUE(higherHalfRegion.start, alignValue);

    ASSERT(higherHalfRegion.start <= alignedUpValue);
    ASSERT(alignedUpValue <= higherHalfRegion.end);
    ASSERT(higherHalfRegion.end - (alignedUpValue + size) <
           higherHalfRegion.start);

    higherHalfRegion.start = alignedUpValue;
    U64 result = higherHalfRegion.start;

    higherHalfRegion.start += size;
    return result;
}
