#ifndef ABSTRACTION_TEXT_CONVERTER_BASE_H
#define ABSTRACTION_TEXT_CONVERTER_BASE_H

#include "shared/text/string.h"
#include "shared/types/array-types.h"
#include "shared/types/numeric.h"

[[nodiscard]] String stringWithMinSize(String data, U8 minSize, U8_a tmp);
[[nodiscard]] String stringWithMinSizeDefault(String data, U8 minSize);

[[nodiscard]] String stringToString(String data);

[[nodiscard]] String charToString(char data, U8_a tmp);
[[nodiscard]] String charToStringDefault(char data);

[[nodiscard]] String boolToString(bool data);

[[nodiscard]] String ptrToString(void *data, U8_a tmp);
[[nodiscard]] String ptrToStringDefault(void *data);

[[nodiscard]] String U64ToString(U64 data, U8_a tmp);
[[nodiscard]] String U64ToStringDefault(U64 data);

[[nodiscard]] String I64ToString(I64 data, U8_a tmp);
[[nodiscard]] String I64ToStringDefault(I64 data);

// clang-format off
#define STRING_CONVERTER_BASE \
        String: stringToString,                                                \
        char: charToStringDefault,                                             \
        bool: boolToString,                                                    \
        void *: ptrToStringDefault,                                            \
        U8 *: ptrToStringDefault,                                              \
        I8 *: ptrToStringDefault,                                              \
        U16 *: ptrToStringDefault,                                             \
        I16 *: ptrToStringDefault,                                             \
        U32 *: ptrToStringDefault,                                             \
        I32 *: ptrToStringDefault,                                             \
        U64 *: ptrToStringDefault,                                             \
        I64 *: ptrToStringDefault,                                             \
        U8: U64ToStringDefault,                                                \
        I8: I64ToStringDefault,                                                \
        U16: U64ToStringDefault,                                               \
        I16: I64ToStringDefault,                                               \
        U32: U64ToStringDefault,                                               \
        I32: I64ToStringDefault,                                               \
        U64: U64ToStringDefault,                                               \
        I64: I64ToStringDefault,

// clang-format on

#endif
