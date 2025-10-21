#ifndef SHARED_TEXT_STRING_H
#define SHARED_TEXT_STRING_H

#include "shared/macros.h"
#include "shared/types/numeric.h"

typedef struct {
    U8 *buf;
    U32 len;
} String;

static constexpr String EMPTY_STRING = ((String){0, 0});
#define STRING(s) ((String){(U8 *)(s), sizeof(s) - 1})
#define STRING_LEN(s, len) ((String){(U8 *)(s), len})
#define STRING_PTRS(begin, end)                                                \
    ((String){(U8 *)(begin), (U32)(((U64)(end)) - ((U64)(begin)))})

[[nodiscard]] bool stringEquals(String a, String b);
[[nodiscard]] bool stringContainsChar(String s, U8 ch);
[[nodiscard]] String stringSplit(String s, U8 token, U32 from);

typedef struct {
    String string;
    U32 pos;
} StringIter;

#define STRING_TOKENIZE(_string, stringIter, token, startingPosition)          \
    for ((stringIter) = (StringIter){.pos = (startingPosition)};               \
         ((stringIter).pos < (_string).len) &&                                 \
         ((stringIter).string = stringSplit(_string, token, (stringIter).pos), \
         1);                                                                   \
         (stringIter).pos += (stringIter).string.len + 1)

typedef struct {
    U32 from;
} OccurrenceStart;
[[nodiscard]] I64 stringOccurrenceOfChar_(String s, U8 ch, U32 from);
#define stringOccurrenceOfChar(string, ch, ...)                                      \
    ({                                                                         \
        OccurrenceStart MACRO_VAR(occurrenceStart) =                           \
            (OccurrenceStart){.from = 0, __VA_ARGS__};                         \
        stringOccurrenceOfChar_(string, ch, MACRO_VAR(occurrenceStart).from);        \
    })

[[nodiscard]] I64 stringLastOccurrenceOfChar(String s, U8 ch);

#endif
