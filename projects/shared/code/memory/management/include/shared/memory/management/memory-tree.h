#ifndef SHARED_MEMORY_MANAGEMENT_MEMORY_TREE_H
#define SHARED_MEMORY_MANAGEMENT_MEMORY_TREE_H

#include "shared/types/array-types.h"

typedef struct {
    void_max_a nodes;
    void *tree;
    voidPtr_max_a freeList;
} TreeWithFreeList;

#endif
