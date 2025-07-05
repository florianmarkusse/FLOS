#include "shared/memory/management/management.h"

#include "abstraction/interrupts.h"
#include "abstraction/memory/manipulation.h"
#include "efi-to-kernel/kernel-parameters.h"
#include "shared/maths.h"
#include "shared/memory/allocator/arena.h"
#include "shared/memory/management/page.h"
#include "shared/memory/sizes.h"
#include "shared/trees/red-black/memory-manager.h"

MemoryAllocator virtualMA;
MemoryAllocator physicalMA;

void insertRedBlackNodeMMAndAddToFreelist(RedBlackNodeMM **root,
                                          RedBlackNodeMM *newNode,
                                          RedBlackNodeMMPtr_max_a *freeList) {
    InsertResult insertResult = insertRedBlackNodeMM(root, newNode);

    for (U64 i = 0; (i < RED_BLACK_MM_MAX_POSSIBLE_FREES_ON_INSERT) &&
                    insertResult.freed[i];
         i++) {
        freeList->buf[freeList->len] = insertResult.freed[i];
        freeList->len++;
    }
}

RedBlackNodeMM *getRedBlackNodeMM(RedBlackNodeMMPtr_max_a *freeList,
                                  Arena *arena) {
    RedBlackNodeMM *result;
    if (freeList->len > 0) {
        result = freeList->buf[freeList->len - 1];
        freeList->len--;
    } else {
        result = NEW(arena, RedBlackNodeMM);
    }

    return result;
}

static void insertMemory(Memory memory, MemoryAllocator *allocator) {
    RedBlackNodeMM *newNode =
        getRedBlackNodeMM(&allocator->freeList, &allocator->arena);
    newNode->memory = memory;

    insertRedBlackNodeMMAndAddToFreelist(&allocator->tree, newNode,
                                         &allocator->freeList);
}

static RedBlackNodeMM *getMemoryAllocation(MemoryAllocator *allocator,
                                           U64 bytes) {
    RedBlackNodeMM *availableMemory =
        deleteAtLeastRedBlackNodeMM(&allocator->tree, bytes);
    if (!availableMemory) {
        if (allocator == &physicalMA) {
            interruptNoMorePhysicalMemory();
        } else {
            interruptNoMoreVirtualMemory();
        }
    }
    return availableMemory;
}

void freeVirtualMemory(Memory memory) { insertMemory(memory, &virtualMA); }

void freePhysicalMemory(Memory memory) { insertMemory(memory, &physicalMA); }

static U64 alignedToTotal(U64 bytes, U64 align) { return bytes + align - 1; }

static void handleRemovedAllocator(RedBlackNodeMM *availableMemory,
                                   Memory memoryUsed,
                                   MemoryAllocator *allocator) {
    U64 beforeResultBytes = memoryUsed.start - availableMemory->memory.start;
    U64 afterResultBytes =
        availableMemory->memory.bytes - (beforeResultBytes + memoryUsed.bytes);

    if (beforeResultBytes && afterResultBytes) {
        availableMemory->memory.bytes = beforeResultBytes;
        (void)insertRedBlackNodeMM(&allocator->tree, availableMemory);

        insertMemory((Memory){.start = memoryUsed.start + memoryUsed.bytes,
                              .bytes = afterResultBytes},
                     allocator);
    } else if (beforeResultBytes) {
        availableMemory->memory.bytes = beforeResultBytes;
        (void)insertRedBlackNodeMM(&allocator->tree, availableMemory);
    } else if (afterResultBytes) {
        availableMemory->memory =
            (Memory){.start = memoryUsed.start + memoryUsed.bytes,
                     .bytes = afterResultBytes};
        (void)insertRedBlackNodeMM(&allocator->tree, availableMemory);
    } else {
        allocator->freeList.buf[allocator->freeList.len] = availableMemory;
        allocator->freeList.len++;
    }
}

static void *allocAlignedMemory(U64 bytes, U64 align,
                                MemoryAllocator *allocator) {
    RedBlackNodeMM *availableMemory =
        getMemoryAllocation(allocator, alignedToTotal(bytes, align));
    U64 result = ALIGN_UP_VALUE(availableMemory->memory.start, align);
    handleRemovedAllocator(
        availableMemory, (Memory){.start = result, .bytes = bytes}, allocator);
    return (void *)result;
}

void *allocVirtualMemory(U64 bytes, U64 align) {
    return allocAlignedMemory(bytes, align, &virtualMA);
}

void *allocPhysicalMemory(U64 bytes, U64 align) {
    return allocAlignedMemory(bytes, align, &physicalMA);
}

static void initMemoryAllocator(PackedMemoryAllocator *packedMemoryAllocator,
                                MemoryAllocator *allocator) {
    allocator->arena.beg = packedMemoryAllocator->allocator.beg;
    allocator->arena.curFree = packedMemoryAllocator->allocator.curFree;
    allocator->arena.end = packedMemoryAllocator->allocator.end;

    allocator->freeList.buf = packedMemoryAllocator->freeList.buf;
    allocator->freeList.cap = packedMemoryAllocator->freeList.cap;
    allocator->freeList.len = packedMemoryAllocator->freeList.len;

    allocator->tree = packedMemoryAllocator->tree;
}

static constexpr auto ALLOCATOR_MAX_BUFFER_SIZE = 1 * GiB;
static constexpr auto ALLOCATOR_PAGE_SIZE = 4 * KiB;

void initMemoryManagers(PackedMemoryAllocator *physicalMemoryTree,
                        PackedMemoryAllocator *virtualMemoryTree) {
    if (setjmp(physicalMA.arena.jmpBuf)) {
        interruptNoMorePhysicalMemory();
    }
    initMemoryAllocator(physicalMemoryTree, &physicalMA);

    if (setjmp(virtualMA.arena.jmpBuf)) {
        interruptNoMoreVirtualMemory();
    }
    initMemoryAllocator(virtualMemoryTree, &virtualMA);

    RedBlackNodeMM *virtualBufferForPhysical =
        allocVirtualMemory(ALLOCATOR_MAX_BUFFER_SIZE, alignof(RedBlackNodeMM));
    U64 arenaBytesUsed =
        (U64)physicalMA.arena.curFree - (U64)physicalMA.arena.beg;

    U64 mapsToDo = CEILING_DIV_VALUE(arenaBytesUsed, (U64)ALLOCATOR_PAGE_SIZE);
    for (U64 i = 0; i < mapsToDo; i++) {
        handlePageFault((U64)virtualBufferForPhysical +
                        (i * ALLOCATOR_PAGE_SIZE));
    }
    memcpy(virtualBufferForPhysical, physicalMA.arena.beg, arenaBytesUsed);
}

void initVirtualMemoryManager(PackedMemoryAllocator *virtualMemoryTree) {
    // TODO: fix this, we are running out of nodes to use for the memory tree,
    // not out of memory. Use virtual memory setup and then this is a fine
    // interrupt I think.
    if (setjmp(virtualMA.arena.jmpBuf)) {
        interruptNoMoreVirtualMemory();
    }
    initMemoryAllocator(virtualMemoryTree, &virtualMA);
}

void initPhysicalMemoryManager(PackedMemoryAllocator *physicalMemoryTree) {
    if (setjmp(physicalMA.arena.jmpBuf)) {
        interruptNoMorePhysicalMemory();
    }
    initMemoryAllocator(physicalMemoryTree, &physicalMA);
}
