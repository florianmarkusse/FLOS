#include "abstraction/log.h"
#include "efi/error.h"
#include "shared/log.h"
#include "shared/text/string.h"
#include "x86/efi/avx512.h"

bool kernelAVX512Support(bool AVX512SupportByCPU) {
    if (AVX512SupportByCPU) {
        INFO(STRING(
            "Kernel was configured without avx512 support, so not turning "
            "it on.\n"));
    }
    return false;
}
