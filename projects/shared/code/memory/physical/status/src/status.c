#include "shared/memory/physical/status.h"

#include "abstraction/log.h"
#include "shared/log.h"
#include "shared/memory/physical.h"
#include "shared/text/string.h"
#include "shared/trees/red-black/memory-manager.h"
#include "shared/types/numeric.h"

static void preOrder(RedBlackNodeMM *current, U64 *totalValue) {
    if (!current) {
        return;
    }

    preOrder(current->children[RB_TREE_LEFT], totalValue);

    KLOG(current->memory.bytes);
    KLOG(STRING(" "));
    *totalValue += current->memory.bytes;

    preOrder(current->children[RB_TREE_RIGHT], totalValue);
}

void appendPhysicalMemoryManagerStatus() {
    KLOG(STRING("Physical Memory status\n"));
    KLOG(STRING("================\n"));
    U64 totalMemory = 0;
    preOrder(tree, &totalMemory);
    KLOG(STRING("\n================\n"));
    KLOG(STRING("Total memory: "));
    KLOG(totalMemory, NEWLINE);
}
