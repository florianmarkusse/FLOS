#ifndef ABSTRACTION_TEXT_CONVERTER_CONVERTER_H
#define ABSTRACTION_TEXT_CONVERTER_CONVERTER_H

#include "shared/text/string.h"

#include "abstraction/text/converter/base.h"

#ifdef FLOAT_OPERATIONS
#include "abstraction/text/converter/float.h"
#else
#define STRING_CONVERTER_FLOAT
#endif

[[nodiscard]] String noAppend();

// clang-format off
#define STRING_CONVERT(data)                                                \
    _Generic((data),                                                           \
        STRING_CONVERTER_BASE                                                 \
        STRING_CONVERTER_FLOAT                                                \
        default: noAppend)(data)
// clang-format on

#endif
