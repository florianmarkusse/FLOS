#ifndef SHARED_TREES_RED_BLACK_TESTS_CORRECT_H
#define SHARED_TREES_RED_BLACK_TESTS_CORRECT_H

#include "shared/memory/allocator/arena.h"
#include "shared/trees/red-black.h"
#include "shared/types/array-types.h"

void assertRedBlackTreeValid(RedBlackNode *tree, U64_max_a expectedValues,
                             Arena scratch);

void appendRedBlackTreeWithBadNode(RedBlackNode *root, RedBlackNode *badNode);
#endif
