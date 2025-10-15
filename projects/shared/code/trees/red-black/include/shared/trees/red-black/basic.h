#ifndef SHARED_TREES_RED_BLACK_BASIC_H
#define SHARED_TREES_RED_BLACK_BASIC_H

#include "shared/macros.h"
#include "shared/memory/management/definitions.h"
#include "shared/trees/red-black/common.h"

typedef struct RedBlackNodeBasic RedBlackNodeBasic;
struct RedBlackNodeBasic {
    RedBlackNodeBasic *
        children[RB_TREE_CHILD_COUNT]; // NOTE: Keep this as the first elements.
                                       // This is used in the insert so that
                                       // children->[0] and a RedBlackNode* are
                                       // the same location for doing inserts.
                                       // And as polymorphism for some common
                                       // operations.
    RedBlackColor color;               // NOTE: Keep this as the second element
    U64 value;
};

static_assert(OFFSETOF(RedBlackNodeBasic, children) == 0);
static_assert(OFFSETOF(RedBlackNodeBasic, color) ==
              sizeof(RedBlackNodeBasic *) * RB_TREE_CHILD_COUNT);

typedef struct {
    RedBlackNodeBasic *node;
    RedBlackDirection direction;
} BasicNodeVisited;

typedef ARRAY(RedBlackNodeBasic *) RedBlackNodeBasicPtr_a;

typedef MAX_LENGTH_ARRAY(RedBlackNodeBasic) RedBlackNodeBasic_max_a;
typedef MAX_LENGTH_ARRAY(RedBlackNodeBasic *) RedBlackNodeBasicPtr_max_a;

void insertRedBlackNodeBasic(RedBlackNodeBasic **tree,
                             RedBlackNodeBasic *createdNode);
[[nodiscard]] RedBlackNodeBasic *
deleteRedBlackNodeBasic(RedBlackNodeBasic **tree, U64 value);
[[nodiscard]] RedBlackNodeBasic *popRedBlackNodeBasic(RedBlackNodeBasic **tree);
[[nodiscard]] RedBlackNodeBasic *
deleteAtLeastRedBlackNodeBasic(RedBlackNodeBasic **tree, U64 value);
[[nodiscard]] RedBlackNodeBasic *
findGreatestBelowOrEqual(RedBlackNodeBasic **tree, U64 value);

#endif
