#include "abstraction/jmp.h"

int setjmp(JumpBuffer buf) {
    asm volatile("movq (%rsp), %rax;"
                 "movq %rax,  0(%rdi);" // rip
                 "lea 8(%rsp), %rax;"
                 "movq %rax,  8(%rdi);" // rsp
                 "movq %rbp, 16(%rdi);"
                 "movq %rbx, 24(%rdi);"
                 "movq %r12, 32(%rdi);"
                 "movq %r13, 40(%rdi);"
                 "movq %r14, 48(%rdi);"
                 "movq %r15, 56(%rdi);"
                 "xor %eax, %eax;"
                 "ret;");
}

void longjmp(JumpBuffer buf, int ret) {
    asm volatile("movq 56(%rdi), %r15;"
                 "movq 48(%rdi), %r14;"
                 "movq 40(%rdi), %r13;"
                 "movq 32(%rdi), %r12;"
                 "movq 24(%rdi), %rbx;"

                 "movq 16(%rdi), %rbp;"
                 "movq  8(%rdi), %rsp;"
                 "movl %esi, %eax;"
                 "jmp *0(%rdi);"); // rip
}
