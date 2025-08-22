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

void appendToBuffer(U8_a *buffer, String data);
void appendToFlushBuffer(String data, U8 flags);
void appendZeroToFlushBuffer(U32 bytes, U8 flags);
bool flushStandardBuffer();
bool flushBuffer(U8_max_a *buffer);

#define KLOG_APPEND(buffer, data)                                              \
    appendToBuffer(buffer, CONVERT_TO_STRING(data))

#define KLOG_DATA(data, flags)                                                 \
    appendToFlushBuffer(CONVERT_TO_STRING(data), flags)

typedef struct {
    U8 flags;
} StandardLoggingParams;

#define KLOG(data, ...)                                                        \
    ({                                                                         \
        StandardLoggingParams MACRO_VAR(standardLoggingParams) =               \
            (StandardLoggingParams){.flags = 0, __VA_ARGS__};                  \
        appendToFlushBuffer(CONVERT_TO_STRING(data),                           \
                            MACRO_VAR(standardLoggingParams).flags);           \
    })

#define INFO(data, ...) KLOG(data, ##__VA_ARGS__)

#define ERROR(data, ...) KLOG(data, ##__VA_ARGS__)

#define KFLUSH_AFTER                                                           \
    for (U32 MACRO_VAR(i) = 0; MACRO_VAR(i) < 1;                               \
         MACRO_VAR(i) = flushStandardBuffer())

#endif
