#ifndef X86_EFI_TIME_H
#define X86_EFI_TIME_H

#include "shared/types/numeric.h"

static constexpr auto MICROSECONDS_PER_SECOND = 1000000;
[[nodiscard]] U64 timestampFrequencyGet();

#endif
