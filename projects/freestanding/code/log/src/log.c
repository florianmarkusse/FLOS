#include "abstraction/log.h"

#include "abstraction/serial.h"
#include "freestanding/log/init.h"
#include "freestanding/memory/manipulation.h"
#include "freestanding/peripheral/screen.h"
#include "shared/log.h"
#include "shared/maths.h"
#include "shared/text/string.h"
#include "shared/types/array-types.h" // for U8_a, uint8_max_a, U8_d_a
#include "shared/types/numeric.h"

// We are going to flush to:
// - The in-memory standin file buffer, this will be replaced by a file
// buffer in the future.
bool flushBuffer(U8_max_a *buffer) {
    flushToScreen(*buffer);

#ifdef SERIAL
    flushToSerial(*buffer);
#endif

    // TODO: Flush to file system here?

    buffer->len = 0;

    return true;
}

bool flushStandardBuffer() { return flushBuffer(&flushBuf); }

void handleFlags(U8 flags) {
    if (flags & NEWLINE) {
        if (flushBuf.len >= flushBuf.cap) {
            flushBuffer(&flushBuf);
        }
        flushBuf.buf[flushBuf.len] = '\n';
        flushBuf.len++;
    }

    if (flags & FLUSH) {
        flushBuffer(&flushBuf);
    }
}

typedef void (*AppendFunction)(void *destination, const void *source,
                               U64 bytes);

static void appendMemcpy(void *destination, const void *source, U64 bytes) {
    memcpy(destination, source, bytes);
}

static void appendMemset(void *destination, const void *source, U64 bytes) {
    (void)source;
    memset(destination, 0, bytes);
}

static void appendData(void *data, U32 len, U8 flags, AppendFunction appender) {
    for (typeof(len) bytesWritten = 0; bytesWritten < len;) {
        // the minimum of size remaining and what is left in the buffer.
        U32 spaceInBuffer = (flushBuf.cap) - flushBuf.len;
        U32 dataToWrite = len - bytesWritten;
        U32 bytesToWrite = MIN(spaceInBuffer, dataToWrite);
        appender(flushBuf.buf + flushBuf.len, data + bytesWritten,
                 bytesToWrite);
        flushBuf.len += bytesToWrite;
        bytesWritten += bytesToWrite;
        if (flushBuf.cap == flushBuf.len) {
            flushBuffer(&flushBuf);
        }
    }

    handleFlags(flags);
}

void appendToFlushBuffer(String data, U8 flags) {
    appendData(data.buf, data.len, flags, appendMemcpy);
}

void appendZeroToFlushBuffer(U32 bytes, U8 flags) {
    appendData(nullptr, bytes, flags, appendMemset);
}
