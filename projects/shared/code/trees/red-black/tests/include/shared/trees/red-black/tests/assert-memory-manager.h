#ifndef SHARED_TREES_RED_BLACK_TESTS_ASSERT_MEMORY_MANAGER_H
#define SHARED_TREES_RED_BLACK_TESTS_ASSERT_MEMORY_MANAGER_H

#include "shared/memory/allocator/arena.h"
#include "shared/trees/red-black/memory-manager.h"

void assertMMRedBlackTreeValid(NodeLocation *nodeLocation, U32 tree,
                               Memory_max_a expectedValues, Arena scratch);

#endif
