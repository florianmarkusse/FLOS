#ifndef SHARED_TREES_RED_BLACK_COMMON_H
#define SHARED_TREES_RED_BLACK_COMMON_H

#include "shared/memory/allocator/arena.h"
#include "shared/types/array-types.h"
#include "shared/types/array.h"
#include "shared/types/numeric.h"

typedef enum { RB_TREE_BLACK = 0, RB_TREE_RED = 1 } RedBlackColor;
typedef enum { RB_TREE_LEFT = 0, RB_TREE_RIGHT = 1 } RedBlackDirection;

static constexpr auto RB_TREE_CHILD_COUNT = 2;
static constexpr auto RB_TREE_MAX_HEIGHT = 128;

RedBlackDirection calculateDirection(U64 value, U64 toCompare);

typedef struct RedBlackNode RedBlackNode;
struct RedBlackNode {
    // NOTE: Packing the children and color into 8 bytes.
    // bit 0  - 30:                                 low child index
    // bit 32 - 62:                                 high child index
    // bit 31/63 (Architecture/Compiler-dependent): node color
    // Don't directly access the metaData field, go through the functions
    U32 metaData[RB_TREE_CHILD_COUNT];
};

#define TREE_ARRAY(T)                                                          \
    struct {                                                                   \
        T *buf;                                                                \
        U32 len;                                                               \
        U32 cap;                                                               \
        U32 elementSizeBytes;                                                  \
    }

#define TREE_WITH_FREELIST(T)                                                  \
    struct {                                                                   \
        U32_max_a freeList;                                                    \
        TREE_ARRAY(T);                                                         \
        U32 rootIndex;                                                         \
    }

typedef TREE_WITH_FREELIST(void) TreeWithFreeList;

typedef void (*RotationUpdater)(TreeWithFreeList *treeWithFreeList,
                                U32 rotationNode, U32 rotationChild);

void treeWithFreeListInit(TreeWithFreeList *result, U32 elementSizeBytes,
                          U32 align, U32 count, Arena *arena);

U32 getIndex(TreeWithFreeList *treeWithFreeList, void *node);
RedBlackNode *getNode(TreeWithFreeList *treeWithFreeList, U32 index);

RedBlackDirection visitedNodeDirectionGet(U32 visitedNodes[RB_TREE_MAX_HEIGHT],
                                          U32 index);
void visitedNodeDirectionSet(U32 visitedNodes[RB_TREE_MAX_HEIGHT], U32 index,
                             RedBlackDirection direction);

U32 visitedNodeIndexGet(U32 visitedNodes[RB_TREE_MAX_HEIGHT], U32 index);
void visitedNodeIndexSet(U32 visitedNodes[RB_TREE_MAX_HEIGHT], U32 position,
                         U32 index);

void setColorWithPointer(RedBlackNode *node, RedBlackColor color);
void setColor(TreeWithFreeList *treeWithFreeList, U32 index,
              RedBlackColor color);

RedBlackColor getColorWithPointer(RedBlackNode *node);
RedBlackColor getColor(TreeWithFreeList *treeWithFreeList, U32 index);

void childNodePointerSet(RedBlackNode *parentNode, RedBlackDirection direction,
                         U32 childIndex);
void childNodeIndexSet(TreeWithFreeList *treeWithFreeList, U32 parent,
                       RedBlackDirection direction, U32 childIndex);

U32 childNodePointerGet(RedBlackNode *parentNode, RedBlackDirection direction);
U32 childNodeIndexGet(TreeWithFreeList *treeWithFreeList, U32 parent,
                      RedBlackDirection direction);

U32 rebalanceInsert(TreeWithFreeList *treeWithFreeList,
                    U32 visitedNodes[RB_TREE_MAX_HEIGHT], U32 len,
                    RedBlackDirection direction,
                    RotationUpdater rotationUpdater);
U32 rebalanceDelete(TreeWithFreeList *treeWithFreeList,
                    U32 visitedNodes[RB_TREE_MAX_HEIGHT], U32 len,
                    RedBlackDirection direction,
                    RotationUpdater rotationUpdater);

void rotateAround(TreeWithFreeList *treeWithFreeList, U32 rotationParent,
                  U32 rotationNode, U32 rotationChild,
                  RedBlackDirection rotationDirection,
                  RedBlackDirection parentToChildDirection);
U32 findAdjacentInSteps(TreeWithFreeList *treeWithFreeList, U32 *visitedNodes,
                        U32 node, RedBlackDirection direction);

#endif
