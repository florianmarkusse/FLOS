#ifndef SHARED_TREES_RED_BLACK_VIRTUAL_MAPPING_MANAGER_H
#define SHARED_TREES_RED_BLACK_VIRTUAL_MAPPING_MANAGER_H

#include "shared/trees/red-black/basic.h"
#include "shared/types/numeric.h"

typedef struct RedBlackVMM {
    RedBlackNodeBasic basic;
    U64 bytes;
    U64 mappingSize;
} RedBlackVMM;

typedef MAX_LENGTH_ARRAY(RedBlackVMM) RedBlackVMM_max_a;
typedef PACKED_MAX_LENGTH_ARRAY(RedBlackVMM) PackedRedBlackVMM_max_a;
typedef MAX_LENGTH_ARRAY(RedBlackVMM *) RedBlackVMMPtr_max_a;
typedef PACKED_MAX_LENGTH_ARRAY(RedBlackVMM *) PackedRedBlackVMMPtr_max_a;

void insertRedBlackVMM(RedBlackVMM **tree, RedBlackVMM *createdNode);
[[nodiscard]] RedBlackVMM *deleteAtLeastRedBlackVMM(RedBlackVMM **tree,
                                                    U64 value);
[[nodiscard]] RedBlackVMM *
findGreatestBelowOrEqualRedBlackVMM(RedBlackVMM **tree, U64 address);

#endif
