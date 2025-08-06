#ifndef SHARED_HASH_TRIE_STRING_U16_MAP_H
#define SHARED_HASH_TRIE_STRING_U16_MAP_H

#include "common-iterator.h" // for TRIE_ITERATOR_HEADER_FILE
#include "shared/macros.h"   // for MACRO_VAR
#include "shared/memory/allocator/arena.h"
#include "shared/text/string.h" // for string
#include "shared/types/numeric.h"

typedef struct {
    String key;
    U16 value;
} trie_stringU16Data;

typedef struct trie_stringU16Map {
    struct trie_stringU16Map *child[4];
    trie_stringU16Data data;
} trie_stringU16Map;

U16 trie_insertStringU16Map(String key, U16 value, trie_stringU16Map **set,
                            Arena *perm);

TRIE_ITERATOR_HEADER_FILE(trie_stringU16Map, trie_stringU16IterNode,
                          trie_stringU16Iterator, trie_stringU16Data,
                          createStringU16Iterator, nextStringU16Iterator)

#define FOR_EACH_TRIE_STRING_U16(element, stringU16Map, scratch)               \
    for (trie_stringU16Iterator * MACRO_VAR(iter) =                            \
             createStringUint16Iterator(stringU16Map, &(scratch));             \
         ;)                                                                    \
        if (((element) = nextStringU16Iterator(MACRO_VAR(iter), &(scratch)))   \
                .value == 0)                                                   \
            break;                                                             \
        else

#endif
