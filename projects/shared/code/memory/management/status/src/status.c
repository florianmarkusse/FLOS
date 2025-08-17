#include "shared/memory/policy/status.h"
#include "shared/memory/management/status.h"

#include "abstraction/log.h"
#include "shared/log.h"
#include "shared/memory/management/management.h"
#include "shared/text/string.h"

static void countAvailable(MMNode *current, U64 *available, U32 *nodes) {
    if (!current) {
        return;
    }

    countAvailable(current->children[RB_TREE_LEFT], available, nodes);

    *available += current->memory.bytes;
    *nodes += 1;

    countAvailable(current->children[RB_TREE_RIGHT], available, nodes);
}

static void appendMemoryManagerStatus(RedBlackMMTreeWithFreeList *allocator,
                                      String name) {
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
    KLOG(STRING("nodes buf: "));
    KLOG((void *)allocator->nodes.buf);
    KLOG(STRING(" len: "));
    KLOG(stringWithMinSizeDefault(CONVERT_TO_STRING(allocator->nodes.len), 3));
    KLOG(STRING(" cap: "));
    KLOG(allocator->nodes.cap, NEWLINE);
}

void appendPhysicalMemoryManagerStatus() {
    appendMemoryManagerStatus(&physicalMA, STRING("[PHYS]"));
}

void appendVirtualMemoryManagerStatus() {
    appendMemoryManagerStatus(&virtualMA, STRING("[VIRT]"));
}

static AvailableMemoryState getAvailableMemory(MMNode *tree) {
    AvailableMemoryState result = {0};
    countAvailable(tree, &result.memory, &result.nodes);
    return result;
}

AvailableMemoryState getAvailablePhysicalMemory() {
    return getAvailableMemory(physicalMA.tree);
}
AvailableMemoryState getAvailableVirtualMemory() {
    return getAvailableMemory(virtualMA.tree);
}
