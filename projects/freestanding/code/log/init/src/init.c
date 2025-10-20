#include "freestanding/log/init.h"

#include "shared/memory/allocator/arena.h"
#include "shared/memory/policy.h"
#include "shared/memory/sizes.h"
#include "shared/types/array-types.h"
#include "shared/types/numeric.h"

static constexpr auto FLUSH_BUFFER_SIZE = (2 * MiB);

U8_max_a loggingFlushBuffer;

void initLogger(Arena *perm) {
    loggingFlushBuffer.buf = NEW(perm, U8, .count = FLUSH_BUFFER_SIZE);
    loggingFlushBuffer.cap = FLUSH_BUFFER_SIZE;
    loggingFlushBuffer.len = 0;
}
