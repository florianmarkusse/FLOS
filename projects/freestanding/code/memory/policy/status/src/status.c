#include "freestanding/memory/policy/status.h"

#include "abstraction/log.h"
#include "freestanding/memory/physical/status.h"
#include "freestanding/memory/virtual/status.h"
#include "shared/log.h"
#include "shared/text/string.h"

void appendMemoryManagementStatus() {
    appendVirtualMemoryManagerStatus();
    appendPhysicalMemoryManagerStatus();
}
