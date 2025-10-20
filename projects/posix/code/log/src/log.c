#include "posix/log.h"

#include <unistd.h>

#include "abstraction/log.h"
#include "posix/memory/manipulation.h"
#include "shared/assert.h"
#include "shared/log.h"
#include "shared/maths.h"
#include "shared/memory/sizes.h"
#include "shared/text/string.h"
#include "shared/types/array-types.h" // for U8_a, uint8_max_a, U8_d_a
#include "shared/types/numeric.h"

static constexpr auto FLUSH_BUFFER_SIZE = (2 * MiB);

static WriteBuffer stdoutBuffer =
    (WriteBuffer){.array = {.buf = (U8[FLUSH_BUFFER_SIZE]){0},
                            .cap = FLUSH_BUFFER_SIZE,
                            .len = 0},
                  .fileDescriptor = STDOUT_FILENO};
static WriteBuffer stderrBuffer =
    (WriteBuffer){.array = {.buf = (U8[FLUSH_BUFFER_SIZE]){0},
                            .cap = FLUSH_BUFFER_SIZE,
                            .len = 0},
                  .fileDescriptor = STDERR_FILENO};

WriteBuffer *getWriteBuffer(BufferType bufferType) {
    if (bufferType == STDOUT) {
        return &stdoutBuffer;
    }
    return &stderrBuffer;
}

typedef struct {
    int fileDescriptor;
} PosixFlushContext;

void bufferFlush(U8_a *buffer, void *flushContext) {
    PosixFlushContext *posixFlushContext = (PosixFlushContext *)flushContext;
    for (typeof(buffer->len) bytesWritten = 0; bytesWritten < buffer->len;) {
        I64 partialBytesWritten =
            write(posixFlushContext->fileDescriptor, buffer->buf + bytesWritten,
                  buffer->len - bytesWritten);
        if (partialBytesWritten < 0) {
            ASSERT(false);
        } else {
            bytesWritten += (U64)partialBytesWritten;
        }
    }
}

void bufferFlushWithWriter(BufferType bufferType) {
    WriteBuffer *writeBuffer = getWriteBuffer(bufferType);
    bufferFlush((U8_a *)(&writeBuffer->array), &writeBuffer->fileDescriptor);
    writeBuffer->array.len = 0;
}

void standardBufferFlush() { bufferFlushWithWriter(STDOUT); }

U8_max_a *flushBufferGet() { return &stdoutBuffer.array; }
FlushFunction flushFunctionGet() { return bufferFlush; }
void *flushContextGet() { return &stdoutBuffer.fileDescriptor; }

static void appendDataToFlushBufferWithWriter(void *data, U32 len, U8 flags,
                                              WriteBuffer *writeBuffer,
                                              AppendFunction appender) {
    appendDataToFlushBuffer(data, len, flags, &writeBuffer->array, appender,
                            &writeBuffer->fileDescriptor);
}

void appendZeroToFlushBufferWithWriter(U32 bytes, U8 flags,
                                       WriteBuffer *writeBuffer) {
    appendDataToFlushBufferWithWriter(nullptr, bytes, flags, writeBuffer,
                                      appendMemset);
}

void appendToFlushBufferWithWriter(String data, U8 flags,
                                   WriteBuffer *writeBuffer) {
    appendDataToFlushBufferWithWriter(data.buf, data.len, flags, writeBuffer,
                                      appendMemcpy);
}

static String ansiColorToCode[COLOR_NUMS] = {
    STRING("\x1b[31m"), STRING("\x1b[32m"), STRING("\x1b[33m"),
    STRING("\x1b[34m"), STRING("\x1b[35m"), STRING("\x1b[36m"),
    STRING("\x1b[0m"),
};

void appendColor(AnsiColor color, BufferType bufferType) {
    WriteBuffer *buffer = getWriteBuffer(bufferType);
    appendToFlushBufferWithWriter(
        isatty(buffer->fileDescriptor) ? ansiColorToCode[color] : EMPTY_STRING,
        0, buffer);
}

void appendColorReset(BufferType bufferType) {
    WriteBuffer *buffer = getWriteBuffer(bufferType);
    appendToFlushBufferWithWriter(isatty(buffer->fileDescriptor)
                                      ? ansiColorToCode[COLOR_RESET]
                                      : EMPTY_STRING,
                                  0, buffer);
}
