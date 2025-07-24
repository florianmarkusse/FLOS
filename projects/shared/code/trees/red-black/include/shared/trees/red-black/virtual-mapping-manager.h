#ifndef SHARED_TREES_RED_BLACK_VIRTUAL_MAPPING_MANAGER_H
#define SHARED_TREES_RED_BLACK_VIRTUAL_MAPPING_MANAGER_H

#include "shared/trees/red-black/basic.h"
#include "shared/types/numeric.h"

typedef struct {
    RedBlackNodeBasic basic;
    U64 bytes;
    U64 mappingSize;
} RedBlackNodeVMM;

typedef MAX_LENGTH_ARRAY(RedBlackNodeVMM) RedBlackVMM_max_a;
typedef MAX_LENGTH_ARRAY(RedBlackNodeVMM *) RedBlackVMMPtr_max_a;

void insertRedBlackNodeVMM(RedBlackNodeVMM **tree,
                           RedBlackNodeVMM *createdNode);
[[nodiscard]] RedBlackNodeVMM *
deleteAtLeastRedBlackNodeVMM(RedBlackNodeVMM **tree, U64 value);
[[nodiscard]] RedBlackNodeVMM *
findGreatestBelowOrEqualRedBlackNodeVMM(RedBlackNodeVMM **tree, U64 address);

#endif
