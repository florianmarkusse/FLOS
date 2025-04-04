#ifndef SHARED_TREES_RED_BLACK_TESTS_CORRECT_H
#define SHARED_TREES_RED_BLACK_TESTS_CORRECT_H

#include "shared/memory/allocator/arena.h"
#include "shared/trees/red-black.h"

void assertRedBlackTreeValid(RedBlackNode *tree, Arena scratch);
#endif
