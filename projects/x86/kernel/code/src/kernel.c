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

    cyclesPerMicroSecond = x86ArchParams->tscFrequencyPerMicroSecond;

    rootPageTable = (VirtualPageTable *)CR3();

    rootPageMetaData.children =
        (PageMetaDataNode *)x86ArchParams->rootPageMetaData.children;
    rootPageMetaData.metaData.entriesMapped =
        x86ArchParams->rootPageMetaData.metaData.entriesMapped;
    rootPageMetaData.metaData.entriesMappedWithSmallerGranularity =
        x86ArchParams->rootPageMetaData.metaData
            .entriesMappedWithSmallerGranularity;
}
