#ifndef SHARED_TREES_RED_BLACK_TESTS_ASSERT_BASIC_H
#define SHARED_TREES_RED_BLACK_TESTS_ASSERT_BASIC_H

#include "shared/memory/allocator/arena.h"
#include "shared/trees/red-black/common.h"
#include "shared/types/array-types.h"

void assertBasicRedBlackTreeValid(NodeLocation *nodeLocation, U32 tree,
                                  U64_max_a expectedValues, Arena scratch);
#endif
