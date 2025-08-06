#include "abstraction/log.h"

#include "abstraction/memory/manipulation.h"

void appendToBuffer(U8_a *buffer, String data) {
    memcpy(&buffer->buf[buffer->len], data.buf, data.len);
    buffer->len += data.len;
}
