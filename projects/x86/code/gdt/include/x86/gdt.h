#ifndef X86_GDT_H
#define X86_GDT_H

#include "shared/types/numeric.h"
#include "x86/memory/definitions.h"
typedef struct __attribute__((packed)) {
    U16 limit;
    U64 base;
} DescriptorTableRegister;

DescriptorTableRegister *prepNewGDT(PhysicalBasePage zeroPages[3]);
void enableNewGDT(DescriptorTableRegister *GDTRegister);

#endif
