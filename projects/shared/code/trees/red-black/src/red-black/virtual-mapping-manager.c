#include "shared/trees/red-black/virtual-mapping-manager.h"
#include "abstraction/memory/virtual/converter.h"

void insertRedBlackVMM(RedBlackVMM **tree, RedBlackVMM *createdNode) {
    insertRedBlackNodeBasic((RedBlackNodeBasic **)tree,
                            (RedBlackNodeBasic *)createdNode);
}

RedBlackVMM *deleteAtLeastRedBlackVMM(RedBlackVMM **tree, U64 value) {
    return (RedBlackVMM *)deleteRedBlackNodeBasic((RedBlackNodeBasic **)tree,
                                                  value);
}

RedBlackVMM *findGreatestBelowOrEqualRedBlackVMM(RedBlackVMM **tree,
                                                 U64 address) {
    return (RedBlackVMM *)findGreatestBelowOrEqual((RedBlackNodeBasic **)tree,
                                                   address);
}
