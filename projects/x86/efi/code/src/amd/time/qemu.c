#include "x86/efi/time.h"

static constexpr auto BASE_FREQUENCY_HERTZ = 3800000000;

U64 timestampFrequencyGet() { return BASE_FREQUENCY_HERTZ; }
