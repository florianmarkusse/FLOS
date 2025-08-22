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
void flushBuffer(U8_a *buffer, void *flushContext) {
    (void)flushContext;

    flushToScreen(*buffer);

#ifdef SERIAL
    flushToSerial(*buffer);
#endif

    // TODO: Flush to file system here?

    buffer->len = 0;
}

void flushStandardBuffer() { flushBuffer((U8_a *)&flushBuf, nullptr); }

static void appendData(void *data, U32 len, U8 flags, U8_max_a *buffer,
                       AppendFunction appender) {
    appendDataCommon(data, len, flags, buffer, appender, flushBuffer, nullptr);
}

void appendToFlushBuffer(String data, U8 flags) {
    appendData(data.buf, data.len, flags, &flushBuf, appendMemcpy);
}

void appendZeroToFlushBuffer(U32 bytes, U8 flags) {
    appendData(nullptr, bytes, flags, &flushBuf, appendMemset);
}
