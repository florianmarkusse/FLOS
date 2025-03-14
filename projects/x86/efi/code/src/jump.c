#include "abstraction/efi.h"

#include "efi-to-kernel/memory/definitions.h"
#include "shared/types/types.h"
#include "x86/efi/gdt.h"
#include "x86/gdt.h"
#include "x86/memory/virtual.h"

void enableNewMemoryMapping() {
    asm volatile("mov %%rax, %%cr3" : : "a"(level4PageTable) : "memory");
}

void toKernel(U64 newStackPointer) {
    asm volatile("movq %0, %%rsp;"
                 "movq %%rsp, %%rbp;"
                 "cld;"
                 "pushq %1;"
                 "retq"
                 :
                 : "r"(newStackPointer), "r"(KERNEL_CODE_START)
                 : "memory");
}

void jumpIntoKernel(U64 newStackPointer) {
    enableNewGDT(gdtDescriptor);
    enableNewMemoryMapping();
    toKernel(newStackPointer);

    __builtin_unreachable();
}
