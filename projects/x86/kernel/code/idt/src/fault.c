#include "abstraction/interrupts.h"

#include "abstraction/log.h"
#include "abstraction/memory/virtual/map.h"
#include "abstraction/thread.h"
#include "shared/enum.h"
#include "shared/log.h"
#include "shared/maths.h"
#include "shared/memory/converter.h"
#include "shared/memory/management/management.h"
#include "shared/memory/management/page.h"
#include "shared/types/numeric.h"
#include "x86/configuration/cpu.h"
#include "x86/fault.h"
#include "x86/memory/definitions.h"

static string faultToString[CPU_FAULT_COUNT] = {CPU_FAULT_ENUM(ENUM_TO_STRING)};

U8 *XSAVESpace;

typedef struct {
    U64 r15;
    U64 r14;
    U64 r13;
    U64 r12;
    U64 r11;
    U64 r10;
    U64 r9;
    U64 r8;

    U64 rbp;
    U64 rdi;
    U64 rsi;
    U64 rdx;
    U64 rcx;
    U64 rbx;
    U64 rax;

    Fault interruptNumber;
    U64 errorCode;

    U64 rip;
    U64 cs;
    U64 eflags;

    // Not switching privileges, but we are using ISTs, so the rsp and stack
    // segment are pushed onto the stack on interrupt.
    U64 rsp;
    U64 ss;
} Registers;

U64 currentNumberOfPageFaults = 0;

__attribute__((noreturn)) static void kernelPanic(Registers *regs) {
    KFLUSH_AFTER {
        INFO(STRING("We are in an interrupt!!!\n"));
        INFO(STRING("regs:\n"));
        INFO(STRING("interrupt number: "));
        INFO(regs->interruptNumber, NEWLINE);
        INFO(STRING("interrupt: "));
        INFO(faultToString[regs->interruptNumber], NEWLINE);
        INFO(STRING("error code: "));
        INFO(regs->errorCode, NEWLINE);
        INFO(STRING("rip: "));
        INFO((void *)regs->rip, NEWLINE);
        INFO(STRING("rsp: "));
        INFO((void *)regs->rsp, NEWLINE);

        INFO(STRING("\n"));

        INFO(STRING("rax: "));
        INFO((void *)regs->rax, NEWLINE);
        INFO(STRING("rbx: "));
        INFO((void *)regs->rbx, NEWLINE);
        INFO(STRING("rcx: "));
        INFO((void *)regs->rcx, NEWLINE);
        INFO(STRING("rdx: "));
        INFO((void *)regs->rdx, NEWLINE);
        INFO(STRING("rsi: "));
        INFO((void *)regs->rsi, NEWLINE);
        INFO(STRING("rdi: "));
        INFO((void *)regs->rdi, NEWLINE);
        INFO(STRING("rbp: "));
        INFO((void *)regs->rbp, NEWLINE);
        INFO(STRING("r8 : "));
        INFO((void *)regs->r8, NEWLINE);
        INFO(STRING("r9 : "));
        INFO((void *)regs->r9, NEWLINE);
        INFO(STRING("r10: "));
        INFO((void *)regs->r10, NEWLINE);
        INFO(STRING("r11: "));
        INFO((void *)regs->r11, NEWLINE);
        INFO(STRING("r12: "));
        INFO((void *)regs->r12, NEWLINE);
        INFO(STRING("r13: "));
        INFO((void *)regs->r13, NEWLINE);
        INFO(STRING("r14: "));
        INFO((void *)regs->r14, NEWLINE);
        INFO(STRING("r15: "));
        INFO((void *)regs->r15, NEWLINE);
    }

    hangThread();

    __builtin_unreachable();
}

void faultHandler(Registers *regs) {
    if (regs->interruptNumber == FAULT_PAGE_FAULT) {
        currentNumberOfPageFaults++;
        PageFaultResult pageFaultResult = handlePageFault(CR2());
        switch (pageFaultResult) {
        case PAGE_FAULT_RESULT_MAPPED: {
            return;
        }
        case PAGE_FAULT_RESULT_STACK_OVERFLOW: {
            KFLUSH_AFTER { INFO(STRING("Stack overflow detected!\n")); }
            kernelPanic(regs);
        }
        }
    } else {
        kernelPanic(regs);
    }
}
