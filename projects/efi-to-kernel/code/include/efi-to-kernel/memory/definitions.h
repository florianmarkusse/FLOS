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

#include "abstraction/memory/virtual/converter.h"
#include "shared/memory/sizes.h"

static constexpr auto KERNEL_STACK_SIZE = 2 * MiB;
static constexpr auto KERNEL_STACK_ALIGNMENT =
    16; // NOTE: Sys-V ABI requires 16-byte aligned stack.

#endif
