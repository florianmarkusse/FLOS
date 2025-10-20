#include "abstraction/interrupts.h"
#include "abstraction/log.h"
#include "efi/error.h"
#include "shared/log.h"
#include "shared/text/string.h"

void interruptPhysicalMemory() {
    EXIT_WITH_MESSAGE {
        ERROR(STRING("Interrupts: No More Physical Memory\n"));
    }

    __builtin_unreachable();
}
void interruptVirtualMemory() {
    EXIT_WITH_MESSAGE { ERROR(STRING("Interrupts: No More Virtual Memory\n")); }
    __builtin_unreachable();
}

void interruptVirtualMemoryMapper() {
    EXIT_WITH_MESSAGE {
        ERROR(STRING("Interrupts: No More Virtual Memory Mapper\n"));
    }
    __builtin_unreachable();
}

void interruptBuffer() {
    EXIT_WITH_MESSAGE { ERROR(STRING("Interrupts: No More Buffer\n")); }
    __builtin_unreachable();
}

void interruptUnexpectedError() {
    EXIT_WITH_MESSAGE { ERROR(STRING("Interrupts: Unexpected Error\n")); }
    __builtin_unreachable();
}
