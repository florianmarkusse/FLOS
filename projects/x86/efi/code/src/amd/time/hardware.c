#include "x86/configuration/cpu.h"
#include "x86/efi/time.h"

static constexpr U32 PSTATE0_CORE = 0xC0010064;

static constexpr auto AMD_BASE_FREQUENCY_HERTZ = 5000000;

U64 timestampFrequencyGet() {
    U64 pState0 = rdmsr(PSTATE0_CORE);

    U64 result = AMD_BASE_FREQUENCY_HERTZ * (pState0 & 0x7FF);
    return result;
}
