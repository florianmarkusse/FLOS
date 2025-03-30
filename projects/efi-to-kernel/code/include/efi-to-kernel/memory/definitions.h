#ifndef EFI_TO_KERNEL_MEMORY_DEFINITIONS_H
#define EFI_TO_KERNEL_MEMORY_DEFINITIONS_H

// virtual memory layout
// ============ LOWER HALF
// 0x0
// ...
// 0z0000FFFFFFFFFFFF
// ============ HIGHER HALF
// 0xFFFF800000000000
// ...
// 0xFFFFFFFFFFFFFFFF

#include "shared/memory/sizes.h"

static constexpr auto LOWER_HALF_END = 0x0000FFFFFFFFFFFF;

static constexpr auto HIGHER_HALF_START = 0xFFFF800000000000;

static constexpr auto KERNEL_STACK_SIZE = 2 * MiB;
static constexpr auto KERNEL_STACK_ALIGNMENT =
    16; // NOTE: Sys-V ABI requires 16-byte aligned stack.

// NOTE: Kernel is statically linked using mcmodel=kernel, so requires to be at
// -2 GiB. If you change this value, change it in the linker script too!
static constexpr auto KERNEL_CODE_START = 0xFFFFFFFF80000000;
static constexpr auto KERNEL_CODE_MAX_SIZE = 2 * GiB;

#endif
