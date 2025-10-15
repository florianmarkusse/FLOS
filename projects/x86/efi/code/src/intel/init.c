#include "x86/efi/init.h"
#include "abstraction/log.h"
#include "efi/error.h"
#include "shared/log.h"

void processorVersionCheck(Manufacturer manufacturer,
                           CPUIDResult *restrict CPUIDMaxAndManufacturer,
                           CPUIDResult *restrict processorInfoAndFeatureBits) {
    (void)processorInfoAndFeatureBits;

    manufacturerCheck(manufacturer, INTEL);
    CPUIDCheck(CPUIDMaxAndManufacturer->eax, PROCESSOR_AND_BUS_FREQ);
}
