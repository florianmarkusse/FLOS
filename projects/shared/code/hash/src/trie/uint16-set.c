#include "shared/assert.h"                    // for ASSERT
#include "shared/hash/hashes.h"               // for hash16_xm3
#include "shared/hash/trie/common-iterator.h" // for TRIE_ITERATOR_SOURCE...
#include "shared/hash/trie/u16-set.h"
#include "shared/memory/allocator/arena.h"
#include "shared/memory/allocator/macros.h"
#include "shared/types/numeric.h"

bool trie_insertU16Set(U16 key, trie_U16Set **set, Arena *perm) {
    ASSERT(key != 0);
    for (U16 hash = hashU16(key); *set != nullptr; (hash = (U16)(hash << 2))) {
        if (key == (*set)->data) {
            return false;
        }
        set = &(*set)->child[hash >> 14];
    }
    *set = NEW(perm, trie_U16Set, .flags = ZERO_MEMORY);
    (*set)->data = key;
    return true;
}

TRIE_ITERATOR_SOURCE_FILE(trie_U16Set, trie_U16IterNode, trie_U16Iterator, U16,
                          createU16Iterator, nextU16Iterator)
