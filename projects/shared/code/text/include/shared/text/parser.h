#ifndef SHARED_TEXT_PARSER_H
#define SHARED_TEXT_PARSER_H

#include "shared/text/string.h"
#include "shared/types/numeric.h"
[[nodiscard]] U32 parseU32(String value, U8 base);
[[nodiscard]] U16 parseU16(String value, U8 base);

#endif
