#include "shared/memory/policy/status.h"
#include "shared/memory/management/status.h"

#include "abstraction/log.h"
#include "shared/log.h"
#include "shared/memory/management/management.h"
#include "shared/text/string.h"

static void countAvailable(RedBlackNodeMM *current, U64 *available,
                           U64 *nodes) {
    if (!current) {
        return;
    }

    countAvailable(current->children[RB_TREE_LEFT], available, nodes);

    *available += current->memory.bytes;
    *nodes += 1;

    countAvailable(current->children[RB_TREE_RIGHT], available, nodes);
}

static void appendMemoryManagerStatus(MemoryAllocator *allocator, string name) {
    KLOG(name);
    AvailableMemoryState result = {0};
    countAvailable(allocator->tree, &result.memory, &result.nodes);
    KLOG(STRING(" mem: "));
    KLOG(stringWithMinSizeDefault(CONVERT_TO_STRING(result.memory), 16));
    KLOG(STRING(" nodes: "));
    KLOG(stringWithMinSizeDefault(CONVERT_TO_STRING(result.nodes), 3));
    KLOG(STRING("freelist size: "));
    KLOG(stringWithMinSizeDefault(CONVERT_TO_STRING(allocator->freeList.len),
                                  3));
    KLOG(STRING("arena beg: "));
    KLOG(allocator->arena.beg);
    KLOG(STRING(" current free: "));
    KLOG(allocator->arena.curFree);
    KLOG(STRING(" end: "));
    KLOG(allocator->arena.end, NEWLINE);
}

void appendPhysicalMemoryManagerStatus() {
    appendMemoryManagerStatus(&physical, STRING("[PHYS]"));
}

void appendVirtualMemoryManagerStatus() {
    appendMemoryManagerStatus(&virt, STRING("[VIRT]"));
}

static AvailableMemoryState getAvailableMemory(RedBlackNodeMM *tree) {
    AvailableMemoryState result = {0};
    countAvailable(tree, &result.memory, &result.nodes);
    return result;
}

AvailableMemoryState getAvailablePhysicalMemory() {
    return getAvailableMemory(physical.tree);
}
AvailableMemoryState getAvailableVirtualMemory() {
    return getAvailableMemory(virt.tree);
}
