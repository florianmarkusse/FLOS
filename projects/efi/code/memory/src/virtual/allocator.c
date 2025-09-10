#include "abstraction/memory/virtual/allocator.h"

#include "abstraction/log.h"
#include "abstraction/memory/manipulation.h"
#include "abstraction/memory/virtual/map.h"

#include "efi/error.h"
#include "efi/globals.h"
#include "efi/memory/physical.h"
#include "shared/log.h"
#include "shared/memory/allocator/macros.h"
#include "shared/types/numeric.h"

void *getZeroedMemoryForVirtual(VirtualAllocationType type) {
    StructReq structReq = virtualStructReqs[type];
    void *result = NEW(&globals.kernelPermanent, U8, .count = structReq.bytes,
                       .align = structReq.align, .flags = ZERO_MEMORY);
    return result;
}

void freeZeroedMemoryForVirtual(U64 address, VirtualAllocationType type) {
    (void)address;
    (void)type;
    EXIT_WITH_MESSAGE { ERROR(STRING("Not required for EFI implementation!")); }
}
