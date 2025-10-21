#include "shared/trees/red-black/virtual-mapping-manager.h"
#include "abstraction/memory/virtual/converter.h"

void VMMNodeInsert(VMMNode **tree, VMMNode *createdNode) {
    redBlackNodeBasicInsert((RedBlackNodeBasic **)tree,
                            (RedBlackNodeBasic *)createdNode);
}

VMMNode *VMMNodeDelete(VMMNode **tree, U64 value) {
    return (VMMNode *)redBlackNodeBasicDelete((RedBlackNodeBasic **)tree,
                                              value);
}

VMMNode *VMMNodeFindGreatestBelowOrEqual(VMMNode **tree, U64 address) {
    return (VMMNode *)redBlackNodeBasicFindGreatestBelowOrEqual((RedBlackNodeBasic **)tree,
                                               address);
}
