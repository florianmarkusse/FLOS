#ifndef ABSTRACTION_JMP_H
#define ABSTRACTION_JMP_H

#ifdef X86
typedef void *JumpBuffer[3];
#else
#error ABSTRACTION_JMP_H
#endif

__attribute__((naked, returns_twice)) int setjmp(JumpBuffer buf);
__attribute__((naked, noreturn)) void longjmp(JumpBuffer buf, int ret);

#endif
