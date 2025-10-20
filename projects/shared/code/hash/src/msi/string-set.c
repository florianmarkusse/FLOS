#include "shared/hash/msi/string-set.h"
#include "shared/hash/hashes.h"     // for hashStringDjb2
#include "shared/hash/msi/common.h" // for MSIIndex

bool MSIStringInsert(String string, U64 hash, MSIString *index) {
    for (U32 i = (U32)hash;;) {
        i = MSIIndex(hash, index->exp, i);
        if (index->buf[i].len == 0) {
            index->len++;
            index->buf[i] = string;
            return true;
        } else if (stringEquals(index->buf[i], string)) {
            return false;
        }
    }
}

bool MSIStringContains(String string, U64 hash, MSIString *index) {
    for (U32 i = (U32)hash;;) {
        i = MSIIndex(hash, index->exp, i);
        if (index->buf[i].len == 0) {
            return false;
        } else if (stringEquals(index->buf[i], string)) {
            return true;
        }
    }
}

HashComparisonStatus MSIStringSetEquals(MSIString *restrict set1,
                                         MSIString *restrict set2) {
    if (set1->len != set2->len) {
        return HASH_COMPARISON_DIFFERENT_SIZES;
    }

    String element;
    FOR_EACH_MSI_STRING(element, set1) {
        if (!MSIStringContains(element, stringSkeetoHash(element), set2)) {
            return HASH_COMPARISON_DIFFERENT_CONTENT;
        }
    }

    FOR_EACH_MSI_STRING(element, set2) {
        if (!MSIStringContains(element, stringSkeetoHash(element), set1)) {
            return HASH_COMPARISON_DIFFERENT_CONTENT;
        }
    }

    return HASH_COMPARISON_SUCCESS;
}
