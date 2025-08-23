#ifndef SHARED_MEMORY_MANAGEMENT_MEMORY_TREE_H
#define SHARED_MEMORY_MANAGEMENT_MEMORY_TREE_H

#include "shared/types/array-types.h"

typedef struct {
    void_max_a nodes;
    void *tree;
    voidPtr_max_a freeList;
} TreeWithFreeList;

void *getNodeFromTreeWithFreeList(voidPtr_max_a *freeList, void_max_a *nodes,
                                  U32 elementSize);

#endif
