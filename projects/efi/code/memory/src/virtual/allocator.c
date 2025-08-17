#include "abstraction/memory/virtual/allocator.h"

#include "abstraction/log.h"
#include "abstraction/memory/manipulation.h"
#include "abstraction/memory/virtual/map.h"

#include "efi/error.h"
#include "efi/memory/physical.h"
#include "shared/types/numeric.h"

void *getZeroedMemoryForVirtual(VirtualAllocationType type) {
    StructReq structReq = virtualStructReqs[type];
    if (structReq.align > UEFI_PAGE_SIZE) {
        EXIT_WITH_MESSAGE {
            ERROR(STRING("Requested alignment larger than possible!"));
        }
    }
    void *result = (void *)allocateBytesInUefiPages(structReq.bytes, true);
    memset(result, 0, structReq.bytes);
    return result;
}

void freeZeroedMemoryForVirtual([[maybe_unused]] U64 address,
                                [[maybe_unused]] VirtualAllocationType type) {
    EXIT_WITH_MESSAGE { ERROR(STRING("Not required for EFI implementation!")); }
}
