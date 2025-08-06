#ifndef ABSTRACTION_TEXT_CONVERTER_BASE_H
#define ABSTRACTION_TEXT_CONVERTER_BASE_H

#include "shared/text/string.h"
#include "shared/types/array-types.h"
#include "shared/types/numeric.h"

String stringWithMinSize(String data, U8 minSize, U8_a tmp);
String stringWithMinSizeDefault(String data, U8 minSize);

String stringToString(String data);

String charToString(char data, U8_a tmp);
String charToStringDefault(char data);

String boolToString(bool data);

String ptrToString(void *data, U8_a tmp);
String ptrToStringDefault(void *data);

String U64ToString(U64 data, U8_a tmp);
String U64ToStringDefault(U64 data);

String I64ToString(I64 data, U8_a tmp);
String I64ToStringDefault(I64 data);

// clang-format off
#define CONVERT_TO_STRING_BASE \
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
