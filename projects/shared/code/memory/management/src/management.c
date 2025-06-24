#include "shared/memory/management/management.h"

#include "abstraction/interrupts.h"
#include "efi-to-kernel/kernel-parameters.h"
#include "shared/maths/maths.h"
#include "shared/memory/allocator/arena.h"
#include "shared/trees/red-black/memory-manager.h"

MemoryAllocator virt;
MemoryAllocator physical;

void insertRedBlackNodeMMAndAddToFreelist(RedBlackNodeMM **root,
                                          RedBlackNodeMM *newNode,
                                          RedBlackNodeMMPtr_a *freeList) {
    InsertResult insertResult = insertRedBlackNodeMM(root, newNode);

    for (U64 i = 0; (i < RED_BLACK_MM_MAX_POSSIBLE_FREES_ON_INSERT) &&
                    insertResult.freed[i];
         i++) {
        freeList->buf[freeList->len] = insertResult.freed[i];
        freeList->len++;
    }
}

RedBlackNodeMM *getRedBlackNodeMM(RedBlackNodeMMPtr_a *freeList, Arena *arena) {
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
        if (allocator == &physical) {
            interruptNoMorePhysicalMemory();
        } else {
            interruptNoMoreVirtualMemory();
        }
    }
    return availableMemory;
}

void freeVirtualMemory(Memory memory) { insertMemory(memory, &virt); }

void freePhysicalMemory(Memory memory) { insertMemory(memory, &physical); }

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
    return allocAlignedMemory(bytes, align, &virt);
}

void *allocPhysicalMemory(U64 bytes, U64 align) {
    return allocAlignedMemory(bytes, align, &physical);
}

static void initMemoryAllocator(PackedMemoryAllocator *packedMemoryAllocator,
                                MemoryAllocator *allocator) {
    allocator->arena.beg = packedMemoryAllocator->allocator.beg;
    allocator->arena.curFree = packedMemoryAllocator->allocator.curFree;
    allocator->arena.end = packedMemoryAllocator->allocator.end;

    allocator->freeList.buf = packedMemoryAllocator->freeList.buf;
    allocator->freeList.len = packedMemoryAllocator->freeList.len;

    allocator->tree = packedMemoryAllocator->tree;
}

void initVirtualMemoryManager(PackedMemoryAllocator *virtualMemoryTree) {
    if (setjmp(virt.arena.jmp_buf)) {
        interruptNoMoreVirtualMemory();
    }
    initMemoryAllocator(virtualMemoryTree, &virt);
}

void initPhysicalMemoryManager(PackedMemoryAllocator *physicalMemoryTree) {
    if (setjmp(physical.arena.jmp_buf)) {
        interruptNoMorePhysicalMemory();
    }
    initMemoryAllocator(physicalMemoryTree, &physical);
}
