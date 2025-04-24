#include "x86/fault/print/test.h"

#include "abstraction/log.h"
#include "abstraction/text/converter/converter.h"
#include "shared/text/string.h"
#include "shared/types/numeric.h"
#include "x86/cpu/status/core.h"
#include "x86/fault.h"

void appendExpectedInterrupt(Fault fault) {
    KLOG(STRING("Missing interrupt\n"));
    appendFault(fault);
    KLOG(STRING("\n"));
}

void appendInterrupts(bool *expectedFaults, bool *actualFaults) {
    KLOG(STRING("Interrupts Table\n"));
    for (U64 i = 0; i < CPU_FAULT_COUNT; i++) {
        appendFault(i);
        KLOG(STRING("\tExpected: "));
        KLOG(stringWithMinSizeDefault(
            expectedFaults[i] ? STRING("ON") : STRING("OFF"), 3));
        KLOG(STRING("\tActual: "));
        KLOG(stringWithMinSizeDefault(
            actualFaults[i] ? STRING("ON") : STRING("OFF"), 3));
        if (expectedFaults[i] != actualFaults[i]) {
            KLOG(STRING("\t!!!"));
        }
        KLOG(STRING("\n"));
    }
}
