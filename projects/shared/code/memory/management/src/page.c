#include "shared/memory/management/page.h"
#include "abstraction/memory/virtual/map.h"
#include "shared/maths.h"
#include "shared/memory/converter.h"
#include "shared/memory/management/management.h"
#include "shared/memory/sizes.h"

static U64 pageSizeFromVMM(U64 faultingAddress) { return 4 * KiB; }

void handlePageFault(U64 faultingAddress) {
    U64 pageSizeForFault = pageSizeFromVMM(faultingAddress);

    U64 startingMap = ALIGN_DOWN_VALUE(faultingAddress, pageSizeForFault);
    U64 pageSizeToUse = pageSizeFitting(startingMap, pageSizeForFault);

    // NOTE: when starting to use SMP, I should first check if this memory
    // is now mapped before doing this.
    // In the context of mapping and unmapping. It may be interesting to
    // mark which cores have accessed which memory so we can limit the
    // flushPage calls to all cores.

    U64 mapsToDo = divideByPowerOf2(pageSizeForFault, pageSizeToUse);
    for (U64 i = 0; i < mapsToDo; i++) {
        U8 *address = allocPhysicalMemory(pageSizeToUse, pageSizeToUse);
        mapPage(startingMap + (i * pageSizeToUse), (U64)address, pageSizeToUse);
    }
}
