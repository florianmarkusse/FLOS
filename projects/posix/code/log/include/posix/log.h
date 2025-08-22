#ifndef POSIX_LOG_H
#define POSIX_LOG_H

#include "abstraction/log.h"
#include "abstraction/text/converter/converter.h"
#include "shared/macros.h"
#include "shared/text/string.h"
#include "shared/types/array-types.h"
#include "shared/types/numeric.h"

typedef struct {
    U8_max_a array;
    int fileDescriptor;
} WriteBuffer;
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

bool appendToFlushBufferWithWriter(String data, U8 flags, WriteBuffer *buffer);
bool appendZeroToFlushBufferWithWriter(U32 bytes, U8 flags,
                                       WriteBuffer *buffer);
bool flushBufferWithWriter(WriteBuffer *buffer);
bool flushBufferWithFileDescriptor(int fileDescriptor, U8 *buffer, U32 size);

bool appendColor(AnsiColor color, BufferType bufferType);
bool appendColorReset(BufferType bufferType);

WriteBuffer *getWriteBuffer(BufferType bufferType);

typedef struct {
    WriteBuffer *buffer;
    U8 flags;
} PosixLoggingParams;

#define PLOG_DATA(data, ...)                                                   \
    ({                                                                         \
        PosixLoggingParams MACRO_VAR(posixLoggingParams) =                     \
            (PosixLoggingParams){                                              \
                .buffer = getWriteBuffer(STDOUT), .flags = 0, __VA_ARGS__};    \
        appendToFlushBufferWithWriter(CONVERT_TO_STRING(data),                 \
                                      MACRO_VAR(posixLoggingParams).flags,     \
                                      MACRO_VAR(posixLoggingParams).buffer);   \
    })

#define PLOG(data, ...) PLOG_DATA(data, ##__VA_ARGS__)
#define PINFO(data, ...) PLOG_DATA(data, ##__VA_ARGS__)
#define PERROR(data, ...)                                                      \
    PLOG_DATA(data, .buffer = getWriteBuffer(STDERR), ##__VA_ARGS__)

#define PFLUSH_TO(bufferType) flushBufferWithWriter(getWriteBuffer(bufferType))

#define PFLUSH_AFTER(bufferType)                                               \
    for (U64 MACRO_VAR(i) = 0; MACRO_VAR(i) < 1;                               \
         MACRO_VAR(i) = (PFLUSH_TO(bufferType), 1))

#endif
