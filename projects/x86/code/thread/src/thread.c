#include "abstraction/thread.h"

void threadHang() { asm volatile("cli; hlt;"); }
