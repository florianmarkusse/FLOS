#include "abstraction/memory/virtual/allocator.h"

#include "abstraction/memory/manipulation.h"
#include "abstraction/memory/virtual/map.h"
#include "shared/memory/management/management.h"
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
    void *result = allocPhysicalMemory(bytes, align);
    memset(result, 0, bytes);
    return result;
}

void freeZeroedMemoryForVirtual(U64 address, VirtualAllocationType type) {
    U64 bytes;
    switch (type) {
    case VIRTUAL_PAGE_TABLE_ALLOCATION: {
        bytes = getVirtualMemoryMappingBytes();
        break;
    }
    case META_DATA_PAGE_ALLOCATION: {
        bytes = getVirtualMemoryMetaDataBytes();
        break;
    }
    }

    freePhysicalMemory((Memory){.start = address, .bytes = bytes});
}
