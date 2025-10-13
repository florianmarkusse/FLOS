#ifndef X86_EFI_INIT_H
#define X86_EFI_INIT_H

#include "x86/configuration/cpu.h"

void processorVersionCheck(CPUIDResult *CPUIDMaxAndManufacturer,
                           CPUIDResult *processorInfoAndFeatureBits);

#endif
