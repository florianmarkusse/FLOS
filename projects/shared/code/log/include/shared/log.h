#ifndef SHARED_LOG_H
#define SHARED_LOG_H

#include "abstraction/text/converter/converter.h"
#include "shared/text/string.h"
#include "shared/types/array-types.h"

static constexpr auto NEWLINE = 0x01;
static constexpr auto FLUSH = 0x02;

typedef void (*AppendFunction)(void *destination, const void *source,
                               U64 bytes);

void appendMemcpy(void *destination, const void *source, U64 bytes);
void appendMemset(void *destination, const void *source, U64 bytes);

void appendToBuffer(U8_a *buffer, String data);
void appendToFlushBuffer(String data, U8 flags);
void appendZeroToFlushBuffer(U32 bytes, U8 flags);

void appendDataToFlushBuffer(void *data, U32 len, U8 flags, U8_max_a *buffer,
                             AppendFunction appender, void *flushContext);

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

#endif
