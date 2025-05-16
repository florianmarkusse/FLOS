#include "abstraction/kernel.h"

#include "abstraction/interrupts.h"
#include "efi-to-kernel/kernel-parameters.h"
#include "shared/types/numeric.h"
#include "x86/configuration/cpu.h"
#include "x86/efi-to-kernel/params.h"
#include "x86/memory/definitions.h"
#include "x86/memory/virtual.h"
#include "x86/time.h"

void archInit(void *archParams) {
    X86ArchParams *x86ArchParams = (X86ArchParams *)archParams;
    initIDT();
    //    // TODO: [X86] I need to enable NMIs here also again!
    //

    rootPageTable = (VirtualPageTable *)CR3();
    rootPageMetaData = *x86ArchParams->rootPageMetaData;
    cyclesPerMicroSecond = x86ArchParams->tscFrequencyPerMicroSecond;
}
