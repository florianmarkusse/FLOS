#ifndef SHARED_TREES_RED_BLACK_VIRTUAL_MAPPING_MANAGER_H
#define SHARED_TREES_RED_BLACK_VIRTUAL_MAPPING_MANAGER_H

#include "shared/trees/red-black/basic.h"
#include "shared/types/numeric.h"

typedef struct {
    RedBlackNodeBasic basic;
    U64 bytes;
    U64_pow2 mappingSize;
} VMMNode;

typedef MAX_LENGTH_ARRAY(VMMNode) RedBlackVMM_max_a;
typedef MAX_LENGTH_ARRAY(VMMNode *) RedBlackVMMPtr_max_a;

void insertVMMNode(VMMNode **tree, VMMNode *createdNode);
[[nodiscard]] VMMNode *deleteVMMNode(VMMNode **tree, U64 value);
[[nodiscard]] VMMNode *findGreatestBelowOrEqualVMMNode(VMMNode **tree,
                                                       U64 address);

#endif
