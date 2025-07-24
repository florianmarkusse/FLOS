#include "shared/trees/red-black/virtual-mapping-manager.h"
#include "abstraction/memory/virtual/converter.h"

void insertVMMNode(VMMNode **tree, VMMNode *createdNode) {
    insertRedBlackNodeBasic((RedBlackNodeBasic **)tree,
                            (RedBlackNodeBasic *)createdNode);
}

VMMNode *deleteVMMNode(VMMNode **tree, U64 value) {
    return (VMMNode *)deleteRedBlackNodeBasic((RedBlackNodeBasic **)tree,
                                              value);
}

VMMNode *findGreatestBelowOrEqualVMMNode(VMMNode **tree, U64 address) {
    return (VMMNode *)findGreatestBelowOrEqual((RedBlackNodeBasic **)tree,
                                               address);
}
