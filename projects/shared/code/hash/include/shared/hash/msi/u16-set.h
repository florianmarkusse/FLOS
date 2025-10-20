#ifndef SHARED_HASH_MSI_U16_SET_H
#define SHARED_HASH_MSI_U16_SET_H

#include "common.h"        // for MSI_SET
#include "shared/macros.h" // for MACRO_VAR
#include "shared/types/numeric.h"

typedef MSI_SET(U16) MSIU16;

[[nodiscard]] bool MSIU16Insert(U16 value, U64 hash, MSIU16 *index);

/**
 * Assumes you know what hash function was used in this hash set. If you use the
 * wrong hash, you get wrong answers!!!
 */
[[nodiscard]] bool MSIU16Contains(U16 value, U64 hash, MSIU16 *index);

#define FOR_EACH_MSI_UINT16(element, msiSet)                                   \
    for (U64 MACRO_VAR(_index) = 0; MACRO_VAR(_index) < (1 << (msiSet)->exp);  \
         ++MACRO_VAR(_index))                                                  \
        if (((element) = (msiSet)->buf[MACRO_VAR(_index)]) != 0)

#endif
