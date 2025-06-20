#include "abstraction/memory/virtual/allocator.h"
#include "abstraction/interrupts.h"
#include "freestanding/memory/virtual/allocator.h"

#include "abstraction/memory/manipulation.h"
#include "abstraction/memory/virtual/map.h"
#include "shared/memory/converter.h"
#include "shared/memory/management/management.h"
#include "shared/memory/sizes.h"
#include "shared/types/numeric.h"

// NOTE: Don't cause page faults in this code. The code is called inside a page
// fault. Any potential page faults must be anticipated and solved manually!

#define MANUAL_DYN_ARRAY(T)                                                    \
    struct {                                                                   \
        T *buf;                                                                \
        U64 len;                                                               \
        U64 maxCap;                                                            \
        U64 mappedBytes;                                                       \
        U64 mappingSize;                                                       \
    }

typedef MANUAL_DYN_ARRAY(void *) voidPtr_man_da;
typedef MANUAL_DYN_ARRAY(U8) U8Ptr_man_da;

typedef struct {
    voidPtr_man_da freeList;
    U8Ptr_man_da batcher;
} BatchAllocator;

static BatchAllocator allocators[VIRTUAL_ALLOCATION_TYPE_COUNT];

// If a dynamic array exceeds this, there are some problems for sure...
static constexpr auto MAX_VIRTUAL_MEMORY = 1 * GiB;
static constexpr auto BATCH_SIZE = 128;

static bool needsNewMapping(U64 elementSizeBytes, U64 elementLen,
                            U64 mappedArrayBytes) {
    U64 totalBytes = elementSizeBytes * elementLen;
    if (totalBytes + elementSizeBytes > mappedArrayBytes) {
        return true;
    }

    return false;
}

void *getZeroedMemoryForVirtual(VirtualAllocationType type) {
    voidPtr_man_da *freeList = &allocators[type].freeList;

    StructReq structReq = virtualStructReqs[type];

    if (freeList->len > 0) {
        // TODO: Shrink the allocator if need be?
        // May cause issues with the freezeroed function below.
        // We will never unmap kernel memory so all the memory used for
        // mapping virtual->physical will come from the batch allocator.
        void *result = freeList->buf[freeList->len - 1];

        // // TODO: Remove this check!!!
        // U8 *address = (U8 *)result;
        // for (U64 i = 0; i < structReq.bytes; i++) {
        //     if (address[i]) {
        //         interruptUnexpectedError();
        //     }
        // }
        freeList->len--;
        return result;
    }

    // TODO: get from buffer instead...

    void *result = allocPhysicalMemory(structReq.bytes, structReq.align);
    memset(result, 0, structReq.bytes);
    return result;
}

// NOTE: When mapping more memory, it will potentially shrink the freelist since
// it requires extra memory for the mapping. So we have to be really careful
// here.
void freeZeroedMemoryForVirtual(U64 address, VirtualAllocationType type) {
    BatchAllocator *allocator = &allocators[type];

    // // TODO: Remove this check!!!
    // StructReq structReq = virtualStructReqs[type];
    // U64 bytes = structReq.bytes;
    // U8 *addressCheck = (U8 *)address;
    // for (U64 i = 0; i < bytes; i++) {
    //     if (addressCheck[i]) {
    //         interruptUnexpectedError();
    //     }
    // }

    if (allocator->freeList.len == allocator->freeList.maxCap) {
        interruptUnexpectedError();
    }

    if (needsNewMapping(sizeof(*(allocator->freeList.buf)),
                        allocator->freeList.len,
                        allocator->freeList.mappedBytes)) {
        void *address =
            getZeroedMemoryForVirtual(VIRTUAL_PAGE_TABLE_ALLOCATION);
        mapPage(((U64)allocator->freeList.buf) +
                    allocator->freeList.mappedBytes,
                (U64)address, allocator->freeList.mappingSize);
        allocator->freeList.mappedBytes += allocator->freeList.mappingSize;
    }

    allocator->freeList.buf[allocator->freeList.len] = (void *)address;
    allocator->freeList.len++;
}

void initFreestandingAllocator() {
    U64 freeListElementSizeBytes = sizeof(*(allocators[0].freeList.buf));
    U64 freeListPageSizeBytes =
        pageSizeEncompassing(BATCH_SIZE * freeListElementSizeBytes);
    for (U64 i = 0; i < VIRTUAL_ALLOCATION_TYPE_COUNT; i++) {
        allocators[i].freeList.buf =
            allocVirtualMemory(MAX_VIRTUAL_MEMORY, freeListPageSizeBytes);
        allocators[i].freeList.len = 0;
        allocators[i].freeList.maxCap =
            MAX_VIRTUAL_MEMORY / freeListElementSizeBytes;
        allocators[i].freeList.mappedBytes = 0;
        allocators[i].freeList.mappingSize = freeListPageSizeBytes;
    }
}

U64 getFreeListLen() {
    U64 result = 0;

    for (U64 i = 0; i < VIRTUAL_ALLOCATION_TYPE_COUNT; i++) {
        result = allocators[i].freeList.len;
    }

    return result;
}
