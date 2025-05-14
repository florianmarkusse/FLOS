#include "abstraction/log.h"
#include "abstraction/memory/physical.h"

#include "efi/error.h"
#include "efi/memory/physical.h"
#include "shared/types/numeric.h"

U64 getPageForMappingVirtualMemory(U64 pageSize, U64 align) {
    if (align > UEFI_PAGE_SIZE) {
        EXIT_WITH_MESSAGE {
            ERROR(STRING("Requested alignment larger than possible!"));
        }
    }
    return allocateBytesInUefiPages(pageSize, true);
}
