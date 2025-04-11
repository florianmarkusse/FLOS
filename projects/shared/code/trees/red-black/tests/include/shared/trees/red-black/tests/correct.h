#ifndef SHARED_TREES_RED_BLACK_TESTS_CORRECT_H
#define SHARED_TREES_RED_BLACK_TESTS_CORRECT_H

#include "shared/memory/allocator/arena.h"
#include "shared/trees/red-black.h"
#include "shared/types/array-types.h"

bool assertRedBlackTreeValid(RedBlackNode *tree, U64_max_a expectedValues,
                             Arena scratch);

void printRedBlackTreeWithBadNode(RedBlackNode *root, RedBlackNode *badNode);
#endif
