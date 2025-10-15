#include "x86/efi/init.h"
#include "abstraction/log.h"
#include "efi/error.h"
#include "shared/log.h"

static constexpr auto MINIMUM_FAMILY_VERSION = 26;

void processorVersionCheck(Manufacturer manufacturer,
                           CPUIDResult *restrict CPUIDMaxAndManufacturer,
                           CPUIDResult *restrict processorInfoAndFeatureBits) {
    manufacturerCheck(manufacturer, AMD);
    CPUIDCheck(CPUIDMaxAndManufacturer->eax, XSAVE_CPU_SUPPORT);

    // TODO: [AMD] required family level >= 26
    U32 processorFamily = (processorInfoAndFeatureBits->eax >> 8) & (0xF);
    if (processorFamily == 0xF) {
        processorFamily += (processorInfoAndFeatureBits->eax >> 20) & (0xFF);
    }
    if (processorFamily < MINIMUM_FAMILY_VERSION) {
        EXIT_WITH_MESSAGE {
            ERROR(STRING("family model: "));
            ERROR(processorFamily);
            ERROR(STRING(" lower than minimum required: "));
            ERROR(MINIMUM_FAMILY_VERSION);
        }
    }
}
