#include "abstraction/thread.h"

void hangThread() { asm volatile("cli; hlt;"); }
