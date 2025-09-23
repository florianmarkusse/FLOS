#include "shared/memory/policy/status.h"
// TODO: Use FRAME_OR_NEXT_PAGE_TABLE mask isntead?

#include "abstraction/log.h"
#include "shared/log.h"
#include "shared/memory/management/status.h"
#include "shared/text/string.h"

void appendMemoryManagementStatus() {
    appendVirtualMemoryManagerStatus();
    appendPhysicalMemoryManagerStatus();
}
