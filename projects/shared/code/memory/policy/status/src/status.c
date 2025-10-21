#include "shared/memory/policy/status.h"

#include "abstraction/log.h"
#include "shared/log.h"
#include "shared/memory/management/status.h"
#include "shared/text/string.h"

void memoryManagementStatusAppend() {
    virtualMemoryManagerStatusAppend();
    physicalMemoryManagerStatusAppend();
}
