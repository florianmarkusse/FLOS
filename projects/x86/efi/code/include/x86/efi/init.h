#ifndef X86_EFI_INIT_H
#define X86_EFI_INIT_H

#include "x86/configuration/cpu.h"

typedef enum { INTEL, AMD } Manufacturer;

void processorVersionCheck(Manufacturer manufacturer,
                           CPUIDResult *restrict CPUIDMaxAndManufacturer,
                           CPUIDResult *restrict processorInfoAndFeatureBits);

void manufacturerCheck(Manufacturer actual, Manufacturer expected);
void CPUIDCheck(U32 actualMax, U32 expectedMax);

#endif
