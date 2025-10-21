#ifndef SHARED_TEXT_PARSER_H
#define SHARED_TEXT_PARSER_H

#include "shared/text/string.h"
#include "shared/types/numeric.h"
[[nodiscard]] U32 U32Parse(String value, U8 base);
[[nodiscard]] U16 U16Parse(String value, U8 base);

#endif
