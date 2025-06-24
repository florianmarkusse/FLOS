#include "abstraction/memory/virtual/allocator.h"
#include "abstraction/interrupts.h"
#include "freestanding/memory/virtual/allocator.h"

#include "abstraction/memory/manipulation.h"
#include "abstraction/memory/virtual/map.h"
#include "shared/maths/maths.h"
#include "shared/memory/converter.h"
#include "shared/memory/management/management.h"
#include "shared/memory/policy.h"
#include "shared/memory/sizes.h"
#include "shared/types/numeric.h"

// NOTE: Don't cause page faults in this code. The code is called inside a page
// fault. Any potential page faults must be anticipated and solved manually!

typedef struct {
    void *buf;

    U64 len;
    U64 maxCap;
    U64 mappedBytes;
    U64 mappingSize;
} ManualDynamicArray;

typedef struct {
    ManualDynamicArray freeList;
    ManualDynamicArray batcher;
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

static void ensureMapped(ManualDynamicArray *manualDynamicArray,
                         U64 elementSizeBytes, bool needsZeroed) {
    if (needsNewMapping(elementSizeBytes, manualDynamicArray->len,
                        manualDynamicArray->mappedBytes)) {
        void *backingAddress = allocateIdentityMemory(
            manualDynamicArray->mappingSize, manualDynamicArray->mappingSize);
        if (needsZeroed) {
            memset(backingAddress, 0, manualDynamicArray->mappingSize);
        }

        mapPage(((U64)manualDynamicArray->buf) +
                    manualDynamicArray->mappedBytes,
                (U64)backingAddress, manualDynamicArray->mappingSize);
        manualDynamicArray->mappedBytes += manualDynamicArray->mappingSize;
    }
}

void *getZeroedMemoryForVirtual(VirtualAllocationType type) {
    ManualDynamicArray *freeList = &allocators[type].freeList;

    StructReq structReq = virtualStructReqs[type];

    if (freeList->len > 0) {
        // TODO: Shrink the allocator if need be?
        // May cause issues with the freezeroed function below.
        // We will never unmap kernel memory so all the memory used for
        // mapping virtual->physical will come from the batch allocator.
        void *result = ((void **)freeList->buf)[freeList->len - 1];

        // // TODO: Remove this check!!!
        U8 *address = (U8 *)result;
        for (U64 i = 0; i < structReq.bytes; i++) {
            if (address[i]) {
                interruptUnexpectedError();
            }
        }
        freeList->len--;
        return result;
    }

    // ManualDynamicArray *batcher = &allocators[type].batcher;
    // ensureMapped(batcher, structReq.bytes, true);
    //
    // void *result = (U8 *)batcher->buf + (structReq.bytes * batcher->len);
    // batcher->len++;

    void *result = allocateIdentityMemory(structReq.bytes, structReq.align);
    memset(result, 0, structReq.bytes);
    return result;
}

// NOTE: When mapping more memory, it will potentially shrink the freelist since
// it requires extra memory for the mapping. So the memory we map in might not
// immediately be used.
void freeZeroedMemoryForVirtual(U64 address, VirtualAllocationType type) {
    ManualDynamicArray *freeList = &allocators[type].freeList;

    // // TODO: Remove this check!!!
    StructReq structReq = virtualStructReqs[type];
    U64 bytes = structReq.bytes;
    U8 *addressCheck = (U8 *)address;
    for (U64 i = 0; i < bytes; i++) {
        if (addressCheck[i]) {
            interruptUnexpectedError();
        }
    }

    if (freeList->len == freeList->maxCap) {
        interruptUnexpectedError();
    }

    ensureMapped(freeList, sizeof(void *), false);

    ((void **)freeList->buf)[freeList->len] = (void *)address;
    freeList->len++;
}

static void initManualDynamicArray(ManualDynamicArray *manualDynamicArray,
                                   U64 elementSizeBytes) {
    U64 pageSizeBytes = pageSizeEncompassing(BATCH_SIZE * elementSizeBytes);

    manualDynamicArray->buf =
        allocVirtualMemory(MAX_VIRTUAL_MEMORY, elementSizeBytes);
    manualDynamicArray->len = 0;
    manualDynamicArray->maxCap = MAX_VIRTUAL_MEMORY / elementSizeBytes;
    manualDynamicArray->mappedBytes = 0;
    manualDynamicArray->mappingSize = pageSizeBytes;
}

void initFreestandingAllocator() {
    U64 freeListElementSizeBytes = sizeof(*(allocators[0].freeList.buf));
    for (U64 i = 0; i < VIRTUAL_ALLOCATION_TYPE_COUNT; i++) {
        initManualDynamicArray(&allocators[i].freeList,
                               freeListElementSizeBytes);
        initManualDynamicArray(&allocators[i].batcher,
                               virtualStructReqs[i].bytes);
    }
}
