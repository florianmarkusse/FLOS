#include "shared/memory/management/status.h"

#include "abstraction/log.h"
#include "shared/log.h"
#include "shared/memory/management/management.h"
#include "shared/text/string.h"

static void preOrder(RedBlackNodeMM *current, U64 *totalValue) {
    if (!current) {
        return;
    }

    preOrder(current->children[RB_TREE_LEFT], totalValue);

    KLOG(STRING("[address="));
    KLOG((void *)current->memory.start);
    KLOG(STRING(", bytes="));
    KLOG(current->memory.bytes);
    KLOG(STRING("] "));
    *totalValue += current->memory.bytes;

    preOrder(current->children[RB_TREE_RIGHT], totalValue);
}

static void appendMemoryManagerStatus(RedBlackNodeMM *tree, string name) {
    KLOG(name);
    KLOG(STRING(" status\n"));
    KLOG(STRING("================\n"));
    U64 totalMemory = 0;
    preOrder(tree, &totalMemory);
    KLOG(STRING("\n================\n"));
    KLOG(STRING("Total memory: "));
    KLOG(totalMemory, NEWLINE);
}

void appendPhysicalMemoryManagerStatus() {
    appendMemoryManagerStatus(physical.tree, STRING("Physical Memory"));
}

void appendVirtualMemoryManagerStatus() {
    appendMemoryManagerStatus(virt.tree, STRING("Virtual Memory"));
}

static void countAvailable(RedBlackNodeMM *current, U64 *available) {
    if (!current) {
        return;
    }

    countAvailable(current->children[RB_TREE_LEFT], available);

    *available += current->memory.bytes;

    countAvailable(current->children[RB_TREE_RIGHT], available);
}

static U64 getAvailableMemory(RedBlackNodeMM *tree) {
    U64 availableMemory = 0;
    countAvailable(tree, &availableMemory);

    return availableMemory;
}

U64 getAvailablePhysicalMemory() { return getAvailableMemory(physical.tree); }
U64 getAvailableVirtualMemory() { return getAvailableMemory(virt.tree); }
