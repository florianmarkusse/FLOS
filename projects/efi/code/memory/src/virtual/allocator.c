#include "abstraction/memory/virtual/allocator.h"

#include "abstraction/log.h"
#include "abstraction/memory/manipulation.h"
#include "abstraction/memory/virtual/map.h"

#include "efi/error.h"
#include "efi/memory/physical.h"
#include "shared/log.h"
#include "shared/types/numeric.h"

void *getZeroedMemoryForVirtual(VirtualAllocationType type) {
    StructReq structReq = virtualStructReqs[type];
    if (structReq.align > UEFI_PAGE_SIZE) {
        EXIT_WITH_MESSAGE {
            ERROR(STRING("Requested alignment larger than possible!"));
        }
    }
    void *result = allocateBytesInUefiPages(structReq.bytes, true);
    memset(result, 0, structReq.bytes);
    return result;
}

void freeZeroedMemoryForVirtual(U64 address, VirtualAllocationType type) {
    (void)address;
    (void)type;
    EXIT_WITH_MESSAGE { ERROR(STRING("Not required for EFI implementation!")); }
}
