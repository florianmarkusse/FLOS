#include "shared/trees/red-black/virtual-mapping-manager.h"
#include "abstraction/memory/virtual/converter.h"

void insertRedBlackNodeVMM(RedBlackNodeVMM **tree,
                           RedBlackNodeVMM *createdNode) {
    insertRedBlackNodeBasic((RedBlackNodeBasic **)tree,
                            (RedBlackNodeBasic *)createdNode);
}

RedBlackNodeVMM *deleteAtLeastRedBlackNodeVMM(RedBlackNodeVMM **tree,
                                              U64 value) {
    return (RedBlackNodeVMM *)deleteRedBlackNodeBasic(
        (RedBlackNodeBasic **)tree, value);
}

RedBlackNodeVMM *findGreatestBelowOrEqualRedBlackNodeVMM(RedBlackNodeVMM **tree,
                                                         U64 address) {
    return (RedBlackNodeVMM *)findGreatestBelowOrEqual(
        (RedBlackNodeBasic **)tree, address);
}
