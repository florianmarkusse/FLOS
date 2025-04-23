#include "shared/memory/virtual/status.h"

#include "abstraction/log.h"
#include "shared/log.h"
#include "shared/memory/virtual.h"
#include "shared/text/string.h"

static void appendVirtualRegionStatus(Memory region) {
    KLOG(STRING("Start: "));
    KLOG((void *)region.start, NEWLINE);
    KLOG(STRING("Bytes: "));
    KLOG((void *)region.bytes, NEWLINE);
}

void appendVirtualMemoryManagerStatus() {
    KLOG(STRING("Available Virtual Memory\n"));
    KLOG(STRING("Lower half (0x0000_000000000000):\n"));
    appendVirtualRegionStatus(lowerHalfRegion);
    KLOG(STRING("Higher half(0xFFFF_000000000000):\n"));
    appendVirtualRegionStatus(higherHalfRegion);
}
