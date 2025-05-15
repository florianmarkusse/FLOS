#include "abstraction/interrupts.h"
#include "efi-to-kernel/kernel-parameters.h"
#include "shared/types/numeric.h"
#include "x86/configuration/cpu.h"
#include "x86/memory/definitions.h"
#include "x86/memory/virtual.h"
#include "x86/time.h"

void archInit(PackedArchitectureInit archInit) {
    initIDT();
    //    // TODO: [X86] I need to enable NMIs here also again!
    //

    rootPageTable = (VirtualPageTable *)CR3();
    rootVirtualMetaData =
        (VirtualMetaData *)archInit.rootVirtualMetaDataAddress;
    rootReferenceCount = archInit.rootReferenceCount;
    cyclesPerMicroSecond = archInit.tscFrequencyPerMicroSecond;
}
