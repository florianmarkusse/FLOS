#include "shared/memory/policy/status.h"

#include "abstraction/log.h"
#include "shared/log.h"
#include "shared/memory/physical/status.h"
#include "shared/memory/virtual/status.h"
#include "shared/text/string.h"

void appendMemoryManagementStatus() {
    appendVirtualMemoryManagerStatus();
    appendPhysicalMemoryManagerStatus();
}
