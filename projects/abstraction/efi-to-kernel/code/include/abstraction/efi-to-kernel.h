#ifndef ABSTRACTION_EFI_TO_KERNEL_H
#define ABSTRACTION_EFI_TO_KERNEL_H

// NOTE: Can use constexpr functions maybe when if ever released??? (c26 please)
// clang-format off
#ifdef X86
    #include "x86/efi-to-kernel/params.h"
    #define ARCH_PARAMS_ALIGNMENT alignof(X86ArchParams)
    #define ARCH_PARAMS_SIZE sizeof(X86ArchParams)
#else
    #error "ABSTRACTION_EFI_H"
#endif
// clang-format on

#endif
