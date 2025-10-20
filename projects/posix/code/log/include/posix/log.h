#ifndef POSIX_LOG_H
#define POSIX_LOG_H

#include "abstraction/log.h"
#include "abstraction/text/converter/converter.h"
#include "shared/macros.h"
#include "shared/text/string.h"
#include "shared/types/array-types.h"
#include "shared/types/numeric.h"

typedef enum {
    COLOR_RED,
    COLOR_GREEN,
    COLOR_YELLOW,
    COLOR_BLUE,
    COLOR_MAGENTA,
    COLOR_CYAN,
    COLOR_RESET,
    COLOR_NUMS
} AnsiColor;

typedef enum { STDOUT, STDERR } BufferType;

typedef struct {
    U8_max_a array;
    int fileDescriptor;
} WriteBuffer;

void flushBufferWithWriterAppend(String data, U8 flags,
                                   WriteBuffer *writeBuffer);
void zeroToFlushBufferWithWriterAppend(U32 bytes, U8 flags,
                                       WriteBuffer *writeBuffer);
void bufferWithWriterFlush(BufferType bufferType);

void colorAppend(AnsiColor color, BufferType bufferType);
void colorAppendReset(BufferType bufferType);

[[nodiscard]] WriteBuffer *writeBufferGet(BufferType bufferType);

typedef struct {
    WriteBuffer *writeBuffer;
    U8 flags;
} PosixLoggingParams;

#define PLOG_DATA(data, ...)                                                   \
    ({                                                                         \
        PosixLoggingParams MACRO_VAR(posixLoggingParams) =                     \
            (PosixLoggingParams){.flags = 0,                                   \
                                 .writeBuffer = writeBufferGet(STDOUT),        \
                                 __VA_ARGS__};                                 \
        flushBufferWithWriterAppend(                                         \
            STRING_CONVERT(data), MACRO_VAR(posixLoggingParams).flags,      \
            MACRO_VAR(posixLoggingParams).writeBuffer);                        \
    })

#define PLOG(data, ...) PLOG_DATA(data, ##__VA_ARGS__)
#define PINFO(data, ...) PLOG_DATA(data, ##__VA_ARGS__)
#define PERROR(data, ...)                                                      \
    PLOG_DATA(data, .writeBuffer = writeBufferGet(STDERR), ##__VA_ARGS__)

#define PFLUSH_TO(bufferType) bufferWithWriterFlush(bufferType)

#define PFLUSH_AFTER(bufferType)                                               \
    for (U64 MACRO_VAR(i) = 0; MACRO_VAR(i) < 1;                               \
         MACRO_VAR(i) = (PFLUSH_TO(bufferType), 1))

#endif
