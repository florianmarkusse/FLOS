#include "abstraction/log.h"
#include "efi/error.h"
#include "shared/log.h"
#include "x86/efi/init.h"

void processorVersionCheck(CPUIDResult *CPUIDMaxAndManufacturer,
                           CPUIDResult *processorInfoAndFeatureBits) {
    EXIT_WITH_MESSAGE {
        ERROR(STRING("Exiting in  intel.c"));
        ERROR(STRING("Exiting in  intel.c"));
    }
}
