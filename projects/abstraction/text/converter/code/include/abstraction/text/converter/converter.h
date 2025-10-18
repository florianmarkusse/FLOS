#ifndef ABSTRACTION_TEXT_CONVERTER_CONVERTER_H
#define ABSTRACTION_TEXT_CONVERTER_CONVERTER_H

#include "shared/text/string.h"

#include "abstraction/text/converter/base.h"

#ifdef FLOAT_OPERATIONS
#include "abstraction/text/converter/float.h"
#else
#define CONVERT_TO_STRING_FLOAT
#endif

[[nodiscard]] String noAppend();

// clang-format off
#define CONVERT_TO_STRING(data)                                                \
    _Generic((data),                                                           \
        CONVERT_TO_STRING_BASE                                                 \
        CONVERT_TO_STRING_FLOAT                                                \
        default: noAppend)(data)
// clang-format on

#endif
