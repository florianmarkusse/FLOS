#ifndef SHARED_TREES_RED_BLACK_VIRTUAL_MAPPING_MANAGER_H
#define SHARED_TREES_RED_BLACK_VIRTUAL_MAPPING_MANAGER_H

#include "shared/trees/red-black/basic.h"
#include "shared/types/numeric.h"

typedef struct RedBlackVMM {
    RedBlackNodeBasic basic;
    U64 bytes;
    U64 mappingSize;
} RedBlackVMM;

typedef ARRAY(RedBlackVMM *) RedBlackVMMPtr_a;
typedef PACKED_ARRAY(RedBlackVMM *) PackedRedBlackVMMPtr_a;

void insertRedBlackVMM(RedBlackVMM **tree, RedBlackVMM *createdNode);
[[nodiscard]] RedBlackNodeBasic *deleteRedBlackVMM(RedBlackVMM **tree,
                                                   U64 value);
[[nodiscard]] U64 getPageSizeVMM(RedBlackVMM **tree, U64 address);

#endif
