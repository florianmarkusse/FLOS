#include "abstraction/interrupts.h"
#include "abstraction/log.h"
#include "efi/error.h"
#include "shared/log.h"
#include "shared/text/string.h"

void interruptNoMorePhysicalMemory() {
    EXIT_WITH_MESSAGE {
        ERROR(STRING("Interrupts: No More Physical Memory\n"));
    }

    __builtin_unreachable();
}
void interruptNoMoreVirtualMemory() {
    EXIT_WITH_MESSAGE { ERROR(STRING("Interrupts: No More Virtual Memory\n")); }
    __builtin_unreachable();
}
void interruptUnexpectedError() {
    EXIT_WITH_MESSAGE { ERROR(STRING("Interrupts: Unexpected Error\n")); }
    __builtin_unreachable();
}
