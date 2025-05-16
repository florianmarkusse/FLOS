#ifndef X86_EFI_TO_KERNEL_DEFINITIONS_H
#define X86_EFI_TO_KERNEL_DEFINITIONS_H

#include "shared/memory/sizes.h"

// NOTE: Kernel is statically linked using mcmodel=kernel, so requires to be at
// -2 GiB. If you change this value, change it in the linker script too!
static constexpr auto KERNEL_CODE_START = 0xFFFFFFFF80000000;
static constexpr auto KERNEL_CODE_MAX_SIZE = 2 * GiB;

#endif
