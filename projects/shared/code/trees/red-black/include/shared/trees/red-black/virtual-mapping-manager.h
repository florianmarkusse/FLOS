#ifndef SHARED_TREES_RED_BLACK_VIRTUAL_MAPPING_MANAGER_H
#define SHARED_TREES_RED_BLACK_VIRTUAL_MAPPING_MANAGER_H

#include "shared/memory/management/definitions.h"
#include "shared/trees/red-black/common.h"
#include "shared/types/array-types.h"
#include "shared/types/numeric.h"

typedef struct {
    Memory memory;
    U64_pow2 mappingSize;
} VMMData;

typedef struct {
    RedBlackNode header;
    VMMData data;
} VMMNode;

typedef MAX_LENGTH_ARRAY(VMMNode) VMMNode_max_a;
typedef TREE_WITH_FREELIST(VMMNode) VMMTreeWithFreeList;

VMMNode *getVMMNode(NodeLocation *nodeLocation, U32 index);

void insertVMMNode(VMMTreeWithFreeList *treeWithFreeList, VMMNode *createdNode);
[[nodiscard]] U32 deleteVMMNode(VMMTreeWithFreeList *treeWithFreeList,
                                U64 value);
// TODO: Delete this function? Just used for testing now
[[nodiscard]] U32 deleteAtLeastVMMNode(VMMTreeWithFreeList *treeWithFreeList,
                                       U64 value);
[[nodiscard]] VMMNode *
findGreatestBelowOrEqualVMMNode(VMMTreeWithFreeList *treeWithFreeList,
                                U64 address);

#endif
