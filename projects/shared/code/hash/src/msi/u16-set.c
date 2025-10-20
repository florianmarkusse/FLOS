#include "shared/hash/msi/u16-set.h"
#include "shared/hash/msi/common.h" // for MSIIndex

bool MSIU16Insert(U16 value, U64 hash, MSIU16 *index) {
    for (U32 i = (U32)hash;;) {
        i = MSIIndex(hash, index->exp, i);
        if (index->buf[i] == 0) {
            index->len++;
            index->buf[i] = value;
            return true;
        } else if (index->buf[i] == value) {
            return false;
        }
    }
}

bool MSIU16Contains(U16 value, U64 hash, MSIU16 *index) {
    for (U32 i = (U32)hash;;) {
        i = MSIIndex(hash, index->exp, i);
        if (index->buf[i] == 0) {
            return false;
        } else if (index->buf[i] == value) {
            return true;
        }
    }
}
