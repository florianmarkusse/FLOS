#ifndef SHARED_LOG_H
#define SHARED_LOG_H

#include "abstraction/text/converter/converter.h"
#include "shared/text/string.h"
#include "shared/types/array-types.h"

static constexpr auto NEWLINE = 0x01;
static constexpr auto FLUSH = 0x02;

typedef void (*AppendFunction)(void *restrict destination,
                               void *restrict source, U64 bytes);

void memcpyAppend(void *restrict destination, void *restrict source, U64 bytes);
void memsetAppend(void *restrict destination, void *restrict source, U64 bytes);

void bufferAppend(U8_a *buffer, String data);
void flushBufferAppend(String data, U8 flags);
void zeroToFlushBufferApppend(U32 bytes, U8 flags);

void dataToFlushBufferAppend(void *data, U32 len, U8 flags, U8_max_a *buffer,
                             AppendFunction appender, void *flushContext);

#define KLOG_APPEND(buffer, data) bufferAppend(buffer, STRING_CONVERT(data))

#define KLOG_DATA(data, flags) flushBufferAppend(STRING_CONVERT(data), flags)

typedef struct {
    U8 flags;
} LoggingParamsStandard;

#define KLOG(data, ...)                                                        \
    ({                                                                         \
        LoggingParamsStandard MACRO_VAR(loggingParamsStandard) =               \
            (LoggingParamsStandard){.flags = 0, __VA_ARGS__};                  \
        flushBufferAppend(STRING_CONVERT(data),                                \
                          MACRO_VAR(loggingParamsStandard).flags);             \
    })

#define INFO(data, ...) KLOG(data, ##__VA_ARGS__)

#define ERROR(data, ...) KLOG(data, ##__VA_ARGS__)

#endif
