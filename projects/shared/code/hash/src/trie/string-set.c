#include "shared/hash/trie/string-set.h"
#include "shared/assert.h"                    // for ASSERT
#include "shared/hash/hashes.h"               // for hashStringDjb2
#include "shared/hash/trie/common-iterator.h" // for TRIE_ITERATOR_SOURCE...
#include "shared/memory/allocator/macros.h"
#include "shared/types/numeric.h"

bool trie_insertStringSet(String key, TrieSetString **set, Arena *perm) {
    ASSERT(key.len > 0);
    for (U64 hash = stringSkeetoHash(key); *set != nullptr; hash <<= 2) {
        if (stringEquals(key, (*set)->data)) {
            return false;
        }
        set = &(*set)->child[hash >> 62];
    }
    *set = NEW(perm, TrieSetString, .flags = ZERO_MEMORY);
    (*set)->data = key;
    return true;
}

TRIE_ITERATOR_SOURCE_FILE(TrieSetString, trie_stringIterNode,
                          trie_stringIterator, String, createStringIterator,
                          nextStringIterator)
