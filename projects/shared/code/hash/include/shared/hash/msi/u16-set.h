#ifndef SHARED_HASH_MSI_U16_SET_H
#define SHARED_HASH_MSI_U16_SET_H

#include "common.h"        // for MSI_SET
#include "shared/macros.h" // for MACRO_VAR
#include "shared/types/numeric.h"

typedef MSI_SET(U16) msi_U16;

bool msi_insertU16(U16 value, U64 hash, msi_U16 *index);

/**
 * Assumes you know what hash function was used in this hash set. If you use the
 * wrong hash, you get wrong answers!!!
 */
bool msi_containsU16(U16 value, U64 hash, msi_U16 *index);

#define FOR_EACH_MSI_UINT16(element, msiSet)                                   \
    for (U64 MACRO_VAR(_index) = 0; MACRO_VAR(_index) < (1 << (msiSet)->exp);  \
         ++MACRO_VAR(_index))                                                  \
        if (((element) = (msiSet)->buf[MACRO_VAR(_index)]) != 0)

#endif
