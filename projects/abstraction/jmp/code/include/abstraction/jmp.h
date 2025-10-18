#ifndef ABSTRACTION_JMP_H
#define ABSTRACTION_JMP_H

// clang-format off
#ifdef X86
    #ifdef EFI
        typedef void *JumpBuffer[10];
    #elif defined(FREESTANDING)
        typedef void *JumpBuffer[8];
    #elif defined(POSIX)
        typedef void *JumpBuffer[8];
    #else
        #error "Unknown X86 environment"
    #endif
#else
    #error "ABSTRACTION_JMP_H"
#endif
// clang-format on

[[nodiscard]] __attribute__((naked, returns_twice)) int setjmp(JumpBuffer buf);
__attribute__((naked, noreturn)) void longjmp(JumpBuffer buf, int ret);

#endif
