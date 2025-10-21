#ifndef X86_CONFIGURATION_FEATURES_H
#define X86_CONFIGURATION_FEATURES_H

#include "shared/types/numeric.h"

typedef struct {
    union {
        U32 ecx;
        struct {
            U32 SSE3 : 1;      // Supports Intel Streaming SIMD Extensions 3
            U32 PCLMULQDQ : 1; // Supports PCLMULQDQ instruction
            U32 DTES64 : 1;    // Supports DS area using 64-bit layout
            U32 MONITOR : 1;   // Supports MONITOR/MWAIT feature / instructions
            U32 DS_CPL : 1;    // Supports the extensions to the Debug Store
                               // feature to allow for branch message storage
                               // qualified by CPL
            U32 VMX : 1;       // Supports Virtual Machine Extensions
            U32 SMX : 1;       // Supports Safer Mode Extensions
            U32 EIST : 1;      // Supports Enhanced Intel SpeedStep Techonology
            U32 TM2 : 1;       // Supports Thermal Monitor 2
            U32 SSSE3 : 1; // Supports Supplemental Streaming SIMD Extensions 3
            U32 CNXT_ID : 1; // Supports L1 data cache mode to adaptive or
                             // shared mode
            U32 SDBG : 1; // Supports IA32_DEBUG_INTERFACE_MSR for silicon debug
            U32 FMA : 1;  // Supports FMA extensions using YMM state
            U32 CMPXCHG16B : 1;          // Supports CMPXCHG16B instruction
            U32 xTPR_Update_Control : 1; // Supports changing IA32_MISC_ENABLES
                                         // [bit 23]
            U32 PDCM : 1; // Supports the performance and debug feature
                          // indication in MSR IA32_PERF_CAPABILITIES
            U32 reserved : 1;
            U32 PCID : 1; // Supports PCIDs and that software may set CR4.PCIDE
                          // to 1
            U32 DCA : 1;  // Supports prefetching of data from memory mapped
                          // device
            U32 SSE4_1 : 1;       // Supports SSE4.1
            U32 SSE4_2 : 1;       // Supports SSE4.2
            U32 x2APIC : 1;       // Supports x2APIC
            U32 MOVBE : 1;        // Supports MOVBE instruction
            U32 POPCNT : 1;       // Support POPCNT instruction
            U32 TSC_DEADLINE : 1; // Supports one-shot operation using TSC
                                  // deadline value
            U32 AESNI : 1;        // Supports AESNI instruction extensions
            U32 XSAVE : 1;   // Supports XSAVE/XRSTOR extended states feature,
                             // XSETBV/XGETBV and XCR0
            U32 OSXSAVE : 1; // Indicates OS set CR4.OSXSAVE [bit 18] to enable
                             // XSETBV/XGETBV instructions to access XCR0 and to
                             // support extended state management using
                             // XSAVE/XRSTOR
            U32 AVX : 1;     // Supports AVX instructions on 256-but YMM state,
                         // and three-operand encoding of 256-bit and 128-bit
                         // SIMD instructions
            U32 F16C : 1;   // Supports 10-bit floating-point conversion
                            // instructions
            U32 RDRAND : 1; // Supports RDRAND instruction
            U32 reserved1 : 1;
        };
    };
    union {
        U32 edx;
        struct {
            U32 FPU : 1; // Contains an x87 FPU
            U32 VME : 1; // Virtual 8086 mode enhancements, including CR4.VME
                         // for controlling the feature, CR4.PVI for protected
                         // mode virtual interrupts, software interrupt
                         // indirection, expansion of the TSS with the software
                         // indirection bitmap, and EFLAGS.VIF and EFLAGS.VIP
                         // flags.
            U32 DE : 1;  // Support for I/O breakpoints, including CR4.DE for
                         // controlling the feature, and optional trapping of
                         // accesses to DR4 and DR5
            U32 PSE : 1; // Large pages of size 4 MByte are supported, including
                         // CR4.PSE for controlling the feature, the defined
                         // dirty bit in PDE (Page Directory Entries), optional
                         // reserved bit trapping in CR3, PDEs, and PTEs.
            U32 TSC : 1; // The RDTSC instruction is supported, including
                         // CR4.TSD for controlling privilege.
            U32 MSR : 1; // The RDMSR and WRMSR instructions are supported. Some
                         // of the MSRs are implementation dependent.
            U32 PAE : 1; // Physical addresses greater than 32 bits are
                         // supported: extended page table entry formats, an
                         // extra level in the page translation tables is
                         // defined, 2-MByte pages are supported instead of 4
                         // Mbyte pages if PAE bit is 1. The actual number of
                         // address bits beyond 32 is not defined, and is
                         // implementation specific.
            U32 MCE : 1; // Exception 18 is defined for Machine Checks,
                         // including CR4.MCE for controlling the feature. This
                         // feature does not define the model-specific
                         // implementations of machine-check error logging,
                         // reporting, and processor shutdowns. Machine Check
                         // exception handlers may have to depend on processor
                         // version to do model specific processing of the
                         // exception, or test for the presence of the Machine
                         // Check feature.
            U32 CX8 : 1; // The compare-and-exchange 8 bytes (64 bits)
                         // instruction is supported (implicitly locked and
                         // atomic).
            U32 APIC : 1; // The processor contains an Advanced Programmable
                          // Interrupt Controller (APIC), responding to memory
                          // mapped commands in the physical address range
                          // FFFE0000H to FFFE0FFFH (by default - some
                          // processors permit the APIC to be relocated).
            U32 reserved2 : 1;
            U32 SEP : 1;  // The SYSENTER and SYSEXIT and associated MSRs are
                          // supported.
            U32 MTRR : 1; // MTRRs are supported. The MTRRcap MSR contains
                          // feature bits that describe what memory types are
                          // supported, how many variable MTRRs are supported,
                          // and whether fixed MTRRs are supported.
            U32 PGE : 1;  // The global bit is supported in paging-structure
                         // entries that map a page, indicating TLB entries that
                         // are common to different processes and need not be
                         // flushed. The CR4.PGE bit controls this feature.
            U32 MCA : 1; // The Machine Check Architecture, which provides a
                         // compatible mechanism for error reporting in P6
                         // family, Pentium 4, Intel Xeon processors, and future
                         // processors, is supported. The MCG_CAP MSR contains
                         // feature bits describing how many banks of error
                         // reporting MSRs are supported.
            U32 CMOV : 1; // The conditional move instruction CMOV is supported.
                          // In addition, if x87 FPU is present as indicated by
                          // the CPUID.FPU feature bit, then the FCOMI and FCMOV
                          // instructions are supported
            U32 PAT : 1;  // Page Attribute Table is supported. This feature
                          // augments the Memory Type Range Registers (MTRRs),
                          // allowing an operating system to specify attributes
            // of memory accessed through a linear address on a 4KB
            // granularity.
            U32 PSE_36 : 1; // 4-MByte pages addressing physical memory beyond 4
                            // GBytes are supported with 32-bit paging. This
                            // feature indicates that upper bits of the physical
                            // address of a 4-MByte page are encoded in bits
                            // 20:13 of the page-directory entry. Such physical
                            // addresses are limited by MAXPHYADDR and may be up
                            // to 40 bits in size.
            U32 PSN : 1;    // The processor supports the 96-bit processor
                            // identification number feature and the feature is
                            // enabled.
            U32 CLFSH : 1;  // CLFLUSH Instruction is supported.
            U32 reserved3 : 1;
            U32 DS : 1; // The processor supports the ability to write debug
                        // information into a memory resident buffer. This
                        // feature is used by the branch trace store (BTS) and
                        // precise event-based sampling (PEBS) facilities (see
                        // Chapter 24, “Introduction to Virtual-Machine
                        // Extensions,” in the Intel® 64 and IA-32 Architectures
                        // Software Developer’s Manual, Volume 3C).
            U32 ACPI : 1; // The processor implements internal MSRs that allow
                          // processor temperature to be monitored and processor
                          // performance to be modulated in predefined duty
                          // cycles under software control.
            U32 MMX : 1;  // The processor supports the Intel MMX technology.
            U32 FXSR : 1; // The FXSAVE and FXRSTOR instructions are supported
                          // for fast save and restore of the floating-point
                          // context. Presence of this bit also indicates that
                          // CR4.OSFXSR is available for an operating system to
                          // indicate that it supports the FXSAVE and FXRSTOR
                          // instructions.
            U32 SSE : 1;  // The processor supports the SSE extensions.
            U32 SSE2 : 1; // The processor supports the SSE2 extensions.
            U32 SS : 1;  // The processor supports the management of conflicting
                         // memory types by performing a snoop of its own cache
                         // structure for transactions issued to the bus.
            U32 HTT : 1; // A value of 0 for HTT indicates there is only a
                         // single logical processor in the package and software
                         // should assume only a single APIC ID is reserved. A
                         // value of 1 for HTT indicates the value in
                         // CPUID.1.EBX[23:16] (the Maximum number of
                         // addressable IDs for logical processors in this
                         // package) is valid for the package.
            U32 TM : 1;  // The processor implements the thermal monitor
                         // automatic thermal control circuitry (TCC).
            U32 reserved4 : 1;
            U32 PBE : 1; // The processor supports the use of the FERR#/PBE# pin
                         // when the processor is in the stop-clock state
                         // (STPCLK# is asserted) to signal the processor that
                         // an interrupt is pending and that the processor
                         // should return to normal operation to handle the
                         // interrupt. Bit 10 (PBE enable) in the
                         // IA32_MISC_ENABLE MSR enables this capability.
        };
    };
} BASICCPUFeatures;

void PGEEnable();
void FPUEnable();
void XSAVEEnableAndConfigure(bool supportsAVX512);
void SSEEnable();
void PATConfigure();

#endif
