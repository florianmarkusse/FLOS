#include "abstraction/log.h"
#include "efi/error.h"
#include "shared/log.h"
#include "shared/text/string.h"
#include "x86/efi/avx512.h"

bool kernelAVX512Support(bool AVX512SupportByCPU) {
    if (AVX512SupportByCPU) {
        return true;
    }

    EXIT_WITH_MESSAGE {
        ERROR(STRING(
            "Was built with AVX512 support, but CPU does not support it!\n"));
    }

    __builtin_unreachable();
}
