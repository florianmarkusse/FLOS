#ifndef ABSTRACTION_TEXT_CONVERTER_FLOAT_H
#define ABSTRACTION_TEXT_CONVERTER_FLOAT_H

#include "shared/text/string.h"
#include "shared/types/array-types.h"
#include "shared/types/numeric.h"

[[nodiscard]] String F64ToString(F64 data, U8_a tmp);
[[nodiscard]] String F64ToStringDefault(F64 data);

// clang-format off
#define STRING_CONVERTER_FLOAT                                                \
        F32: F64ToStringDefault,                                               \
        F64: F64ToStringDefault,

// clang-format on

#endif
