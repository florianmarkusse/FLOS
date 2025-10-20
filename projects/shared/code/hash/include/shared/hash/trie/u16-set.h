#ifndef SHARED_HASH_TRIE_U16_SET_H
#define SHARED_HASH_TRIE_U16_SET_H

#include "common-iterator.h" // for TRIE_ITERATOR_HEADER_FILE
#include "shared/macros.h"   // for MACRO_VAR
#include "shared/memory/allocator/arena.h"
#include "shared/types/numeric.h"

typedef struct TrieSetU16 TrieSetU16;
struct TrieSetU16 {
    struct TrieSetU16 *child[4];
    U16 data;
};

[[nodiscard]] bool trieU16Insert(U16 key, TrieSetU16 **set, Arena *perm);

TRIE_ITERATOR_HEADER_FILE(TrieSetU16, trie_U16IterNode, trie_U16Iterator, U16,
                          createU16Iterator, nextU16Iterator)

#define FOR_EACH_TRIE_U16(element, intSet, scratch)                            \
    for (trie_U16Iterator * MACRO_VAR(iter) =                                  \
             createU16Iterator(intSet, &(scratch));                            \
         ;)                                                                    \
        if (((element) = nextU16Iterator(MACRO_VAR(iter), &(scratch))) == 0)   \
            break;                                                             \
        else

#endif
