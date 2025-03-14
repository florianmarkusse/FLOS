#ifndef SHARED_ASSERT_H
#define SHARED_ASSERT_H

#ifdef DEBUG

#if defined(FREESTANDING) || defined(EFI)
// Use set $pc += 2 to resume exection
#define BREAKPOINT asm volatile("1: jmp 1b");
#define ASSERT(c)                                                              \
    if (!(c)) {                                                                \
        asm volatile("1: jmp 1b");                                     \
    }
#elif POSIX

#define BREAKPOINT asm volatile("int3; nop");

#if _MSC_VER
#define ASSERT(c)                                                              \
    if (!(c))                                                                  \
        __debugbreak();
#elif __GNUC__
#define ASSERT(c)                                                              \
    if (!(c)) {                                                                \
        /* __builtin_trap(); */                                                \
        *(volatile int *)0 = 0;                                                \
    }
#else
#define ASSERT(c)                                                              \
    if (!(c))                                                                  \
        *(volatile int *)0 = 0;
#endif
#else
#error "Could not match ENVIRONMENT"
#endif

#else
#define ASSERT(c) ((void)0)
#define BREAKPOINT ((void)0)
#endif

#endif
