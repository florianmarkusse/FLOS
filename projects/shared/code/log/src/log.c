#include "abstraction/log.h"
#include "shared/log.h"

#include "abstraction/memory/manipulation.h"
#include "shared/maths.h"

void appendToBuffer(U8_a *buffer, String data) {
    memcpy(&buffer->buf[buffer->len], data.buf, data.len);
    buffer->len += data.len;
}

void appendMemcpy(void *destination, const void *source, U64 bytes) {
    memcpy(destination, source, bytes);
}

void appendMemset(void *destination, const void *source, U64 bytes) {
    (void)source;
    memset(destination, 0, bytes);
}

void appendDataCommon(void *data, U32 len, U8 flags, U8_max_a *buffer,
                      AppendFunction appender, FlushFunction flusher,
                      void *flushContext) {
    for (U32 bytesWritten = 0; bytesWritten < len;) {
        U32 spaceInBuffer = buffer->cap - buffer->len;
        U32 dataToWrite = len - bytesWritten;
        U32 bytesToWrite = MIN(spaceInBuffer, dataToWrite);

        appender(buffer->buf + buffer->len, (U8 *)data + bytesWritten,
                 bytesToWrite);
        buffer->len += bytesToWrite;
        bytesWritten += bytesToWrite;

        if (buffer->cap == buffer->len) {
            flusher((U8_a *)buffer, flushContext);
        }
    }

    if (flags & NEWLINE) {
        if (buffer->len >= buffer->cap) {
            flusher((U8_a *)buffer, flushContext);
        }
        buffer->buf[buffer->len] = '\n';
        buffer->len++;
    }

    if (flags & FLUSH) {
        flusher((U8_a *)buffer, flushContext);
    }
}
