#include "x86/gdt.h"

void GDTAndSegmentsLoad(DescriptorTableRegister *GDT) {
    asm volatile("lgdt %0;" // Load new Global Descriptor Table

                 "movq $0x08, %%rax;"
                 "pushq %%rax;"           // Push kernel code segment
                 "leaq 1f(%%rip), %%rax;" // Use relative offset for label
                 "pushq %%rax;"           // Push return address
                 "lretq;" // Far return, pop return address into IP,
                          //   and pop code segment into CS
                 "1:"
                 "movw $0x10, %%ax;" // 0x10 = kernel data segment
                 "movw %%ax, %%ds;"  // Load data segment registers
                 "movw %%ax, %%es;"
                 "movw %%ax, %%fs;"
                 "movw %%ax, %%gs;"
                 "movw %%ax, %%ss;"

                 :
                 : "m"(*GDT)
                 : "rax", "memory");
}

void taskRegisterLoad(U16 processorID) {
    U16 tssSelector = processorID * sizeof(TSSDescriptor) + CODE_SEGMENTS_BYTES;
    asm volatile("ltr %0;" : : "r"(tssSelector) : "memory");
}
