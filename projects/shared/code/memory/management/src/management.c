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

static void insertMemory(Memory memory, MemoryAllocator *allocator) {
    RedBlackNodeMM *newNode;
    if (allocator->freeList.len > 0) {
        newNode = allocator->freeList.buf[allocator->freeList.len - 1];
        allocator->freeList.len--;
    } else {
        newNode = NEW(&allocator->arena, RedBlackNodeMM);
    }
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

static void setupArena(PackedArena *packed, Arena *arena) {
    arena->beg = packed->beg;
    arena->curFree = packed->curFree;
    arena->end = packed->end;
}

static void setupFreeList(PackedRedBlackNodeMMPtr_a *packed,
                          RedBlackNodeMMPtr_a *freeList) {
    freeList->buf = packed->buf;
    freeList->len = packed->len;
}

static U64 getRequiredFreeListSize(MemoryAllocator *allocator) {
    U64 redBlackNodeMMsPossibleInAllocator =
        (allocator->arena.end - allocator->arena.beg) /
        sizeof(*(allocator->tree));
    return redBlackNodeMMsPossibleInAllocator * sizeof(allocator->tree);
}

// TODO: Make this equal with physical memory init
void initVirtualMemoryManager(PackedMemoryAllocator virtualMemoryTree) {
    setupArena(&virtualMemoryTree.allocator, &virt.arena);
    if (setjmp(virt.arena.jmp_buf)) {
        interruptNoMoreVirtualMemory();
    }
    setupFreeList(&virtualMemoryTree.freeList, &virt.freeList);

    virt.tree = virtualMemoryTree.tree;
}

// NOTE: Coming into this, All the memory is identity mapped. Having to do some
// boostrapping here.
void initPhysicalMemoryManager(PackedMemoryAllocator physicalMemoryTree) {
    setupArena(&physicalMemoryTree.allocator, &physical.arena);
    if (setjmp(physical.arena.jmp_buf)) {
        interruptNoMorePhysicalMemory();
    }
    setupFreeList(&physicalMemoryTree.freeList, &physical.freeList);

    physical.tree = physicalMemoryTree.tree;
}
