#include "x86/cpu/status/core.h"

#include "abstraction/log.h"
#include "abstraction/text/converter/base.h"
#include "shared/enum.h"
#include "shared/text/string.h"

static string faultToString[CPU_FAULT_COUNT] = {CPU_FAULT_ENUM(ENUM_TO_STRING)};

void appendFault(Fault fault) {
    KLOG(STRING("Fault #: "));
    KLOG(fault);
    KLOG(STRING("\tMsg: "));
    KLOG(stringWithMinSizeDefault(faultToString[fault], 30));
}
