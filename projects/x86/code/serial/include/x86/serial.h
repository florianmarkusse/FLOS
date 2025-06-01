#ifndef X86_SERIAL_H
#define X86_SERIAL_H

#include "shared/types/numeric.h"

static constexpr auto COM1 = 0x3F8;

U8 inb(U16 port);
void outb(U16 port, U8 value);

#endif
