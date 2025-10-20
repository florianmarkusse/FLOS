#include "abstraction/log.h"

#include "abstraction/memory/manipulation.h"
#include "efi/firmware/simple-text-output.h"
#include "efi/globals.h"
#include "shared/log.h"
#include "shared/memory/sizes.h"
#include "shared/text/string.h"
#include "shared/types/array-types.h" // for U8_a, uint8_max_a, U8_d_a
#include "shared/types/numeric.h"

static constexpr auto FLUSH_BUFFER_SIZE = (2 * KiB);
U8_max_a loggingFlushBuffer = (U8_max_a){
    .buf = (U8[FLUSH_BUFFER_SIZE]){0}, .len = 0, .cap = FLUSH_BUFFER_SIZE};

static U16 convertedChar[2] = {0, '\0'};
void bufferFlush(U8_a *buffer, void *flushContext) {
    (void)flushContext;

    for (typeof(buffer->len) i = 0; i < buffer->len; i++) {
        if (buffer->buf[i] == '\n') {
            globals.st->con_out->output_string(globals.st->con_out, u"\r\n");
        } else {
            convertedChar[0] = buffer->buf[i];
            globals.st->con_out->output_string(globals.st->con_out,
                                               convertedChar);
        }
    }

    buffer->len = 0;
}

void standardBufferFlush() {
    bufferFlush((U8_a *)&loggingFlushBuffer, nullptr);
}

U8_max_a *flushBufferGet() { return &loggingFlushBuffer; }
FlushFunction flushFunctionGet() { return bufferFlush; }
void *flushContextGet() { return nullptr; }
