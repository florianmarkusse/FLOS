#ifndef ABSTRACTION_LOG_H
#define ABSTRACTION_LOG_H

#include "abstraction/text/converter/converter.h"
#include "shared/macros.h"
#include "shared/text/string.h"
#include "shared/types/array-types.h"
#include "shared/types/numeric.h"

// NOTE: This is the basic logging implementation that all environments should
// implement if they want to do any sort of logging. Additionally, each
// environment is free to enhance their logging in any way they see fit.

void standardBufferFlush();
void bufferFlush(U8_a *buffer, void *flushContext);

typedef void (*FlushFunction)(U8_a *buffer, void *flushContext);

[[nodiscard]] U8_max_a *flushBufferGet();
[[nodiscard]] FlushFunction flushFunctionGet();
[[nodiscard]] void *flushContextGet();

#define KFLUSH_AFTER                                                           \
    for (U32 MACRO_VAR(i) = 0; MACRO_VAR(i) < 1;                               \
         MACRO_VAR(i) = (standardBufferFlush(), 1))

#endif
