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

#include "efi-to-kernel/kernel-parameters.h"
#include "shared/maths/maths.h"
#include "shared/memory/sizes.h"

static constexpr auto LOWER_HALF_END = 0x0000FFFFFFFFFFFF;

static constexpr auto HIGHER_HALF_START = 0xffff800000000000;

// The idea is to map the kernel to a single 1 page. So we need to find
// portions of physical memory that are part of the same page alignment so
// we can then map that whole portion to a single page. This does not mean
// that the kernel will use up that page of physical memory, however.
static constexpr auto KERNEL_SPACE_START = 0xFFFFFFFFC0000000;

static constexpr auto KERNEL_PARAMS_SIZE = sizeof(KernelParameters);
static constexpr auto KERNEL_PARAMS_ALIGNMENT = alignof(KernelParameters);

static constexpr auto KERNEL_STACK_SIZE = 2 * MiB;
static constexpr auto KERNEL_STACK_ALIGNMENT =
    16; // NOTE: Sys-V ABI requires 16-byte aligned stack.

static constexpr auto KERNEL_CODE_MAX_ALIGNMENT = 2 * MiB;
// NOTE: Kernel is statically linked. If you change this value, change it in the
// linker script too!
static constexpr auto KERNEL_CODE_START = 0xFFFFFFFFFFE00000;

#endif
