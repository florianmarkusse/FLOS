#include "abstraction/memory/management/status.h"

#include "abstraction/log.h"
#include "abstraction/memory/physical/status.h"
#include "abstraction/memory/virtual/status.h"
#include "shared/log.h"
#include "shared/text/string.h"

void appendMemoryManagementStatus() {
    appendVirtualMemoryManagerStatus();
    appendPhysicalMemoryManagerStatus();
}
