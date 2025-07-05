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

U64 getPageSizeVMM(RedBlackVMM **tree, U64 value) {
    RedBlackVMM *result = (RedBlackVMM *)findGreatestBelowOrEqual(
        (RedBlackNodeBasic **)tree, value);
    if (result && result->basic.value + result->bytes > value) {
        return result->mappingSize;
    }

    return SMALLEST_VIRTUAL_PAGE;
}
