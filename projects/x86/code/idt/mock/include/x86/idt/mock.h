#ifndef X86_IDT_MOCK_H
#define X86_IDT_MOCK_H

#include "abstraction/jmp.h"
void initIDTTest(JumpBuffer long_jmp);
bool *getTriggeredFaults();
void resetTriggeredFaults();

bool compareInterrupts(bool *expectedFaults);

#endif
