#include "shared/hash/trie/string-auto-u16-map.h"
#include "shared/assert.h"                    // for ASSERT
#include "shared/hash/hashes.h"               // for hashStringDjb2
#include "shared/hash/trie/common-iterator.h" // for TRIE_ITERATOR_SOURCE...
#include "shared/memory/allocator/macros.h"

StringU16Insert trieStringU16Insert(String key, TrieStringU16 *set,
                                    Arena *perm) {
    StringU16Node **currentNode = &set->node;
    for (U64 hash = stringSkeetoHash(key); *currentNode != nullptr;
         hash <<= 2) {
        if (stringEquals(key, (*currentNode)->data.key)) {
            return (StringU16Insert){.entryIndex = (*currentNode)->data.value,
                                     .wasInserted = false};
        }
        currentNode = &(*currentNode)->child[hash >> 62];
    }
    if (set->nodeCount == U16_MAX) {
        ASSERT(false);
        longjmp(perm->jmpBuf, 1);
    }
    *currentNode = NEW(perm, StringU16Node, .flags = ZERO_MEMORY);
    (*currentNode)->data.key = key;
    set->nodeCount++;
    (*currentNode)->data.value = set->nodeCount;
    return (StringU16Insert){.wasInserted = true,
                             .entryIndex = (set)->nodeCount};
}

U16 trieStringU16Contains(String key, TrieStringU16 *set) {
    StringU16Node **currentNode = &set->node;
    for (U64 hash = stringSkeetoHash(key); *currentNode != nullptr;
         hash <<= 2) {
        if (stringEquals(key, (*currentNode)->data.key)) {
            return (*currentNode)->data.value;
        }
        currentNode = &(*currentNode)->child[hash >> 62];
    }
    return 0;
}

TRIE_ITERATOR_SOURCE_FILE(StringU16Node, trie_stringAutoU16IterNode,
                          trie_stringAutoU16Iterator, StringU16,
                          createStringAutoU16Iterator,
                          nextStringAutoU16Iterator)
