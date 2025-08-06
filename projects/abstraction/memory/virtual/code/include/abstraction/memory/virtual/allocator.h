#ifndef ABSTRACTION_MEMORY_VIRTUAL_ALLOCATOR_H
#define ABSTRACTION_MEMORY_VIRTUAL_ALLOCATOR_H

#include "shared/enum.h"
#include "shared/memory/management/definitions.h"
#include "shared/types/numeric.h"

// NOTE: If ever adding a value to this enum, please check the usages of the
// enum values, it is used below!
#define VIRTUAL_ALLOCATION_TYPE_ENUM(VARIANT)                                  \
    VARIANT(VIRTUAL_PAGE_TABLE_ALLOCATION)                                     \
    VARIANT(META_DATA_PAGE_ALLOCATION)

typedef enum {
    VIRTUAL_ALLOCATION_TYPE_ENUM(ENUM_STANDARD_VARIANT)
} VirtualAllocationType;

static constexpr auto VIRTUAL_ALLOCATION_TYPE_COUNT =
    (0 VIRTUAL_ALLOCATION_TYPE_ENUM(PLUS_ONE));

typedef struct {
    U64 bytes;
    U64 align;
} StructReq;

// NOTE: here as config keys!
extern StructReq virtualStructReqs[VIRTUAL_ALLOCATION_TYPE_COUNT];

void *getZeroedMemoryForVirtual(VirtualAllocationType type);
void freeZeroedMemoryForVirtual(U64 address, VirtualAllocationType type);

#endif
