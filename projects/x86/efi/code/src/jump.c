#include "abstraction/efi.h"

#include "shared/types/numeric.h"
#include "x86/efi/gdt.h"
#include "x86/gdt.h"
#include "x86/memory/virtual.h"

static void enableNewMemoryMapping() {
    asm volatile("mov %%rax, %%cr3" : : "a"(pageTableRoot) : "memory");
}

static void toKernel(U64 newStackPointer, KernelParameters *kernelParams) {
    // NOTE: The Sys V ABI requires rsp to be 16-byte aligned before doing a
    // call instruction to jump to a function. When you do call, it pushes 8
    // bytes on the rsp so the C-compiler assumes that rsp is not 16-byte
    // aligned but only 8-byte aligned. We are not using call, as there is no
    // way we are returning from the kernel entry. So, we set rsp to already be
    // in the state it would be if we would use call instead of jmp.
    U64 alignedStackPointer = newStackPointer - 8;
    asm volatile("movq %0, %%rsp;"
                 "movq %%rsp, %%rbp;"
                 "movq %1, %%rdi;"
                 "cld;"
                 "jmp *%2;"
                 :
                 : "r"(alignedStackPointer), "r"(kernelParams),
                   "r"(kernelCodeStart())
                 : "memory");

    __builtin_unreachable();
}

void kernelJump(U64 newStackPointer, U16 processorID,
                    KernelParameters *kernelParams) {
    GDTAndSegmentsLoad(&gdtDescriptor);
    taskRegisterLoad(processorID);
    enableNewMemoryMapping();
    toKernel(newStackPointer, kernelParams);
}
