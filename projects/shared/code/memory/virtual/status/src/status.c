#include "shared/memory/virtual/status.h"

#include "abstraction/log.h"
#include "shared/log.h"
#include "shared/memory/virtual.h"
#include "shared/text/string.h"

static void appendVirtualRegionStatus(Range region) {
    KLOG(STRING("Start: "));
    KLOG((void *)region.start, NEWLINE);
    KLOG(STRING("end:   "));
    KLOG((void *)region.end, NEWLINE);
}

void appendVirtualMemoryManagerStatus() {
    KLOG(STRING("Available Virtual Memory\n"));
    for (U64 i = 0; i < freeVirtualMemory.len; i++) {
        appendVirtualRegionStatus(freeVirtualMemory.buf[i]);
    }
}
