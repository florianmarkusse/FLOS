#include "abstraction/memory/virtual/allocator.h"

#include "abstraction/log.h"
#include "abstraction/memory/virtual/map.h"

#include "efi/error.h"
#include "efi/memory/physical.h"
#include "shared/types/numeric.h"

void *getZeroedMemoryForVirtual(VirtualAllocationType type) {
    U64 bytes;
    U64 align;
    switch (type) {
    case VIRTUAL_PAGE_TABLE_ALLOCATION: {
        bytes = getVirtualMemoryMappingBytes();
        align = getVirtualMemoryMappingAlignment();
        break;
    }
    case META_DATA_PAGE_ALLOCATION: {
        bytes = getVirtualMemoryMetaDataBytes();
        align = getVirtualMemoryMetaDataAlignment();
        break;
    }
    }
    if (align > UEFI_PAGE_SIZE) {
        EXIT_WITH_MESSAGE {
            ERROR(STRING("Requested alignment larger than possible!"));
        }
    }
    void *result = (void *)allocateBytesInUefiPages(bytes, true);
    memset(result, 0, bytes);
    return result;
}

void freeZeroedMemoryForVirtual([[maybe_unused]] U64 address,
                                [[maybe_unused]] VirtualAllocationType type) {
    EXIT_WITH_MESSAGE { ERROR(STRING("Not required for EFI implementation!")); }
}
