#include "shared/memory/policy/status.h"
#include "shared/memory/management/status.h"

#include "abstraction/log.h"
#include "shared/log.h"
#include "shared/memory/management/management.h"
#include "shared/text/string.h"

static AvailableMemoryState
getAvailableMemory(MMTreeWithFreeList *treeWithFreeList) {
    AvailableMemoryState result = {0};
    if (!(treeWithFreeList->rootIndex)) {
        return result;
    }

    U32 queue[RB_TREE_MAX_HEIGHT];
    queue[0] = treeWithFreeList->rootIndex;
    U32 len = 1;

    while (len > 0) {
        U32 poppedIndex = queue[len - 1];
        len--;
        MMNode *poppedNode = getMMNode(treeWithFreeList, poppedIndex);
        result.nodes++;
        result.memory += poppedNode->data.memory.bytes;

        U32 leftChildIndex =
            childNodePointerGet(&poppedNode->header, RB_TREE_LEFT);
        if (leftChildIndex) {
            queue[len] = leftChildIndex;
            len++;
        }

        U32 rightChildIndex =
            childNodePointerGet(&poppedNode->header, RB_TREE_RIGHT);
        if (rightChildIndex) {
            queue[len] = rightChildIndex;
            len++;
        }
    }

    return result;
}

static void appendMemoryManagerStatus(MMTreeWithFreeList *treeWithFreeList,
                                      String name) {
    INFO(name);
    AvailableMemoryState result = getAvailableMemory(treeWithFreeList);
    INFO(STRING(" mem: "));
    INFO(stringWithMinSizeDefault(CONVERT_TO_STRING(result.memory), 16));
    INFO(STRING(" nodes: "));
    INFO(stringWithMinSizeDefault(CONVERT_TO_STRING(result.nodes), 3));
    INFO(STRING("freelist size: "));
    INFO(stringWithMinSizeDefault(
        CONVERT_TO_STRING(treeWithFreeList->freeList.len), 3));
    INFO(STRING("nodes buf: "));
    INFO((void *)treeWithFreeList->buf);
    INFO(STRING(" len: "));
    INFO(stringWithMinSizeDefault(CONVERT_TO_STRING(treeWithFreeList->len), 3));
    INFO(STRING(" cap: "));
    INFO(treeWithFreeList->cap, .flags = NEWLINE);
}

void appendPhysicalMemoryManagerStatus() {
    appendMemoryManagerStatus(&physicalMA, STRING("[PHYS]"));
}

void appendVirtualMemoryManagerStatus() {
    appendMemoryManagerStatus(&virtualMA, STRING("[VIRT]"));
}

AvailableMemoryState getAvailablePhysicalMemory() {
    return getAvailableMemory(&physicalMA);
}

AvailableMemoryState getAvailableVirtualMemory() {
    return getAvailableMemory(&virtualMA);
}
