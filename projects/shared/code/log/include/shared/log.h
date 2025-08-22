#ifndef SHARED_LOG_H
#define SHARED_LOG_H

#include "shared/types/array-types.h"

static constexpr auto NEWLINE = 0x01;
static constexpr auto FLUSH = 0x02;

typedef void (*FlushFunction)(U8_a *buffer, void *flushContext);

typedef void (*AppendFunction)(void *destination, const void *source,
                               U64 bytes);

void appendMemcpy(void *destination, const void *source, U64 bytes);
void appendMemset(void *destination, const void *source, U64 bytes);

void appendDataCommon(void *data, U32 len, U8 flags, U8_max_a *buffer,
                      AppendFunction appender, FlushFunction flusher,
                      void *flushContext);

#endif
