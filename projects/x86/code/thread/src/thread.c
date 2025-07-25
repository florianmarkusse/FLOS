#include "abstraction/thread.h"

void hangThread() { asm volatile("cli; hlt;"); }

void enableInterrupts() { asm volatile("sti;"); }
