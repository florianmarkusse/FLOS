#include "abstraction/memory/virtual/allocator.h"
#include "abstraction/interrupts.h"

#include "abstraction/memory/manipulation.h"
#include "abstraction/memory/virtual/map.h"
#include "shared/maths.h"
#include "shared/memory/converter.h"
#include "shared/memory/management/management.h"
#include "shared/memory/policy.h"
#include "shared/memory/sizes.h"
#include "shared/types/numeric.h"

// NOTE: Don't cause page faults in this code. The code is called inside a page
// fault. Any potential page faults must be anticipated and solved manually!

void *getZeroedMemoryForVirtual(VirtualAllocationType type) {
    U32 bytes = virtualStructBytes[type];
    void *result = allocateIdentityMemory(bytes);
    memset(result, 0, bytes);

    return result;
}

// NOTE: When mapping more memory, it will potentially shrink the freelist since
// it requires extra memory for the mapping. So the memory we map in might not
// immediately be used.
void freeZeroedMemoryForVirtual(U64 address, VirtualAllocationType type) {
    freeIdentityMemory(
        (Memory){.start = address, .bytes = virtualStructBytes[type]});
}
