#include "abstraction/jmp.h"

__attribute__((naked, returns_twice)) int setjmp(JumpBuffer buf) {
    asm volatile("mov (%rsp), %rax;"
                 "mov %rax,  0(%rcx);" // rip
                 "lea 8(%rsp), %rax;"
                 "mov %rax,  8(%rcx);" // rsp
                 "mov %rbp, 16(%rcx);"
                 "mov %rbx, 24(%rcx);"
                 "mov %rdi, 32(%rcx);"
                 "mov %rsi, 40(%rcx);"
                 "mov %r12, 48(%rcx);"
                 "mov %r13, 56(%rcx);"
                 "mov %r14, 64(%rcx);"
                 "mov %r15, 72(%rcx);"
                 "xor %eax, %eax;"
                 "ret;");
}

__attribute__((naked, noreturn)) void longjmp(JumpBuffer buf, int ret) {
    asm volatile("mov 72(%rcx), %r15;"
                 "mov 64(%rcx), %r14;"
                 "mov 56(%rcx), %r13;"
                 "mov 48(%rcx), %r12;"
                 "mov 40(%rcx), %rsi;"
                 "mov 32(%rcx), %rdi;"
                 "mov 24(%rcx), %rbx;"
                 "mov 16(%rcx), %rbp;"
                 "mov  8(%rcx), %rsp;"
                 "mov %edx, %eax;"
                 "jmp *0(%rcx);");
}
