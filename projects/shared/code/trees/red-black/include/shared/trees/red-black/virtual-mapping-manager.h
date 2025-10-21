#ifndef SHARED_TREES_RED_BLACK_VIRTUAL_MAPPING_MANAGER_H
#define SHARED_TREES_RED_BLACK_VIRTUAL_MAPPING_MANAGER_H

#include "shared/trees/red-black/basic.h"
#include "shared/types/numeric.h"

typedef struct {
    RedBlackNodeBasic basic;
    U64 bytes;
    U64_pow2 mappingSize;
} VMMNode;

typedef ARRAY_MAX_LENGTH(VMMNode) RedBlackVMM_max_a;
typedef ARRAY_MAX_LENGTH(VMMNode *) RedBlackVMMPtr_max_a;

void VMMNodeInsert(VMMNode **tree, VMMNode *createdNode);
[[nodiscard]] VMMNode *VMMNodeDelete(VMMNode **tree, U64 value);
[[nodiscard]] VMMNode *VMMNodeFindGreatestBelowOrEqual(VMMNode **tree,
                                                       U64 address);

#endif
