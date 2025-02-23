#include "shared/text/converter/buffer.h"

#include "shared/memory/sizes.h"
#include "shared/types/array-types.h"

static constexpr auto STRING_CONVERTER_BUF_LEN = 1 * KiB;
U8_a stringConverterBuffer = (U8_a){.buf = (U8[STRING_CONVERTER_BUF_LEN]){0},
                                    .len = STRING_CONVERTER_BUF_LEN};
