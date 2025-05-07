#include "x86/configuration/cpu.h"

U64 rdmsr(U32 msr) {
    U32 edx;
    U32 eax;
    asm volatile("rdmsr" : "=a"(eax), "=d"(edx) : "c"(msr) : "memory");
    return ((U64)edx << 32) | eax;
}

void wrmsr(U32 msr, U64 value) {
    U32 edx = value >> 32;
    U32 eax = (U32)value;
    asm volatile("wrmsr" : : "a"(eax), "d"(edx), "c"(msr) : "memory");
}

void flushTLB() {
    U64 cr3;
    asm volatile("mov %%cr3, %0" : "=r"(cr3)::"memory");
    asm volatile("mov %0, %%cr3" ::"r"(cr3) : "memory");
}

void flushCPUCaches() { asm volatile("wbinvd" ::: "memory"); }

CPUIDResult CPUIDWithSubleaf(U32 leaf, U32 subleaf) {
    CPUIDResult result;
    asm volatile("cpuid"
                 : "=a"(result.eax), "=b"(result.ebx), "=c"(result.ecx),
                   "=d"(result.edx)
                 : "a"(leaf), "c"(subleaf)
                 : "cc");
    return result;
}

CPUIDResult CPUID(U32 leaf) { return CPUIDWithSubleaf(leaf, 0x0); }

void disablePIC() {
    asm volatile("movb $0xFF, %%al;" // Set AL to 0xFF
                 "outb %%al, $0x21;" // Disable master PIC
                 "outb %%al, $0xA1;" // Disable slave PIC
                 :                   // No output operands
                 :                   // No input operands
                 : "eax", "memory"   // Clobbered registers: eax and memory
    );
}

U64 CR3() {
    U64 cr3;
    asm volatile("mov %%cr3, %0" : "=r"(cr3));
    return cr3;
}

U64 CR2() {
    U64 cr2;
    asm volatile("mov %%cr2, %0" : "=r"(cr2));
    return cr2;
}
