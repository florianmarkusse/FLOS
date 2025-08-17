#include "abstraction/serial.h"
#include "x86/serial.h"

U8 inb(U16 port) {
    U8 ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void outb(U16 port, U8 value) {
    asm volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static constexpr auto LINE_STATUS_REGISTER_OFFSET = 5;
static constexpr auto TRANSMITTER_HOLDING_REGISTER_EMPTY = 0b100000;

void flushToSerial(U8_max_a buffer) {
    for (typeof(buffer.len) i = 0; i < buffer.len; i++) {
        while (!((inb(COM1 + LINE_STATUS_REGISTER_OFFSET) &
                  TRANSMITTER_HOLDING_REGISTER_EMPTY))) {
        }
        outb(COM1, buffer.buf[i]);
    }
}
