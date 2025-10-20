#ifndef SHARED_HASH_TRIE_STRING_AUTO_U16_MAP_H
#define SHARED_HASH_TRIE_STRING_AUTO_U16_MAP_H

#include "common-iterator.h" // for TRIE_ITERATOR_HEADER_FILE
#include "shared/memory/allocator/arena.h"
#include "shared/text/string.h" // for string
#include "shared/types/numeric.h"

typedef struct {
    String key;
    U16 value;
} StringU16;

typedef struct StringU16Node StringU16Node;
struct StringU16Node {
    struct StringU16Node *child[4];
    StringU16 data;
};

typedef struct {
    StringU16Node *node;
    // Used as the auto-incrementing key
    U16 nodeCount;
} TrieStringU16;

typedef struct {
    U16 entryIndex;
    bool wasInserted;
} StringU16Insert;

[[nodiscard]] StringU16Insert
trieStringU16Insert(String key, TrieStringU16 *set, Arena *perm);

[[nodiscard]] U16 trieStringU16Contains(String key, TrieStringU16 *set);

TRIE_ITERATOR_HEADER_FILE(StringU16Node, trie_stringAutoU16IterNode,
                          trie_stringAutoU16Iterator, StringU16,
                          createStringAutoU16Iterator,
                          nextStringAutoU16Iterator)

#define FOR_EACH_TRIE_STRING_AUTO_U16(element, stringAutoU16Map, scratch)      \
    for (trie_stringAutoU16Iterator *iter =                                    \
             createStringAutoU16Iterator(stringAutoU16Map, &(scratch));        \
         ;)                                                                    \
        if (((element) = nextStringAutoU16Iterator(iter, &(scratch))).value == \
            0)                                                                 \
            break;                                                             \
        else

#endif
