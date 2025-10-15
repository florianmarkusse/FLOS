#include "abstraction/log.h"
#include "efi/error.h"
#include "shared/log.h"
#include "x86/efi/init.h"

static constexpr CPUIDLeaf BASIC_MAX_REQUIRED = PROCESSOR_AND_BUS_FREQ;

void processorVersionCheck(Manufacturer manufacturer,
                           CPUIDResult *CPUIDMaxAndManufacturer,
                           CPUIDResult *processorInfoAndFeatureBits) {
    (void)processorInfoAndFeatureBits;

    manufacturerCheck(manufacturer, INTEL);
    CPUIDCheck(CPUIDMaxAndManufacturer->eax, PROCESSOR_AND_BUS_FREQ);
}
