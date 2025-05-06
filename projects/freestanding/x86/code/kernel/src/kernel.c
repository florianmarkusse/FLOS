#include "abstraction/interrupts.h"
#include "efi-to-kernel/kernel-parameters.h"
#include "shared/types/numeric.h"
#include "x86/time.h"

void archInit(PackedArchitectureInit archInit) {
    initIDT();
    //    // TODO: [X86] I need to enable NMIs here also again!
    //

    cyclesPerMicroSecond = archInit.cyclesPerMicroSecond;
}
