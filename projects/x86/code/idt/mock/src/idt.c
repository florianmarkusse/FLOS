#include "x86/idt.h"

#include "abstraction/jmp.h"
#include "shared/types/types.h"
#include "x86/fault.h"
#include "x86/idt/mock.h"

static bool triggeredFaults[CPU_FAULT_COUNT];

static void *interruptJumper;

void initIDT() {}

void triggerFault(Fault fault) {
    triggeredFaults[fault] = true;
    longjmp(interruptJumper, 1);
}

void initIDTTest(JumpBuffer long_jmp) { interruptJumper = long_jmp; }

bool *getTriggeredFaults() { return triggeredFaults; }
void resetTriggeredFaults() {
    for (U64 i = 0; i < CPU_FAULT_COUNT; i++) {
        triggeredFaults[i] = false;
    }
}

bool compareInterrupts(bool *expectedFaults) {
    bool *actualFaults = getTriggeredFaults();

    for (U64 i = 0; i < CPU_FAULT_COUNT; i++) {
        if (expectedFaults[i] != actualFaults[i]) {
            return false;
        }
    }

    return true;
}
