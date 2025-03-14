#include "abstraction/jmp.h"

__attribute__((naked, returns_twice)) int setjmp(JumpBuffer buf) {
    asm volatile("movq (%rsp), %rax;"
                 "movq %rax,  0(%rdi);" // rip
                 "lea 8(%rsp), %rax;"
                 "movq %rax,  8(%rdi);" // rsp
                 "movq %rbp, 16(%rdi);" // rbp
                 "xor %eax, %eax;"
                 "ret;");
}

__attribute__((naked, noreturn)) void longjmp(JumpBuffer buf, int ret) {
    asm volatile("movq 16(%rdi), %rbp;" // rbp
                 "movq  8(%rdi), %rsp;" // rsp
                 "movl %esi, %eax;"
                 "jmp *0(%rdi);"); // rip
}
