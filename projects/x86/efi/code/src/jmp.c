#include "abstraction/jmp.h"

__attribute__((naked, returns_twice)) int setjmp(JumpBuffer buf) {
    asm volatile("movq (%rsp), %rax;"
                 "movq %rax,  0(%rcx);" // rip
                 "lea 8(%rsp), %rax;"
                 "movq %rax,  8(%rcx);" // rsp
                 "movq %rbp, 16(%rcx);" // rbp
                 "xor %eax, %eax;"
                 "ret;");
}

__attribute__((naked, noreturn)) void longjmp(JumpBuffer buf, int ret) {
    asm volatile("movq 16(%rcx), %rbp;" // rbp
                 "movq  8(%rcx), %rsp;" // rsp
                 "movl %edx, %eax;"
                 "jmp *0(%rcx);"); // rip
}
