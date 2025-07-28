#ifndef X86_KERNEL_IDT_H
#define X86_KERNEL_IDT_H

#include "x86/fault.h"

extern U8 *XSAVESpace;

__attribute__((noreturn)) void triggerFault(Fault fault);

#endif
