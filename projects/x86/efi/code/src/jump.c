#include "abstraction/efi.h"

#include "efi-to-kernel/memory/definitions.h"
#include "shared/types/numeric.h"
#include "x86/efi/gdt.h"
#include "x86/gdt.h"
#include "x86/memory/virtual.h"

void enableNewMemoryMapping() {
    asm volatile("mov %%rax, %%cr3" : : "a"(rootPageTable) : "memory");
}

void toKernel(U64 newStackPointer, PackedKernelParameters *kernelParams) {
    asm volatile("movq %0, %%rsp;"
                 "movq %%rsp, %%rbp;"
                 "movq %1, %%rdi;"
                 "cld;"
                 "pushq %2;"
                 "retq"
                 :
                 : "r"(newStackPointer), "r"(kernelParams),
                   "r"(KERNEL_CODE_START)
                 : "memory");
}

void jumpIntoKernel(U64 newStackPointer, PackedKernelParameters *kernelParams) {
    enableNewGDT(gdtDescriptor);
    enableNewMemoryMapping();
    toKernel(newStackPointer, kernelParams);

    __builtin_unreachable();
}
