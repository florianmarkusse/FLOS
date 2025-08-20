#include "abstraction/efi.h"
#include "shared/memory/sizes.h"

// NOTE: Kernel is statically linked using mcmodel=kernel, so requires to be at
// -2 GiB. If you change this value, change it in the linker script too!
U64 kernelCodeStart() { return 0xFFFFFFFF80000000; }
U64 kernelCodeSizeMax() { return 2 * GiB; }
U64 kernelVirtualMemoryEnd() { return kernelCodeStart(); }
