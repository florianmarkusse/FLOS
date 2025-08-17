#ifndef SHARED_TEXT_STRING_H
#define SHARED_TEXT_STRING_H

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

bool stringEquals(String a, String b);
String stringCopy(String dest, String src);
U8 getChar(String str, U32 index);
U8 getCharOr(String str, U32 index, I8 or);
U8 *getCharPtr(String str, U32 index);
bool containsChar(String s, U8 ch);
String splitString(String s, U8 token, U32 from);

typedef struct {
    String string;
    U32 pos;
} StringIter;

#define TOKENIZE_STRING(_string, stringIter, token, startingPosition)          \
    for ((stringIter) = (StringIter){.pos = (startingPosition)};               \
         ((stringIter).pos < (_string).len) &&                                 \
         ((stringIter).string = splitString(_string, token, (stringIter).pos), \
         1);                                                                   \
         (stringIter).pos += (stringIter).string.len + 1)

typedef struct {
    U32 from;
} OccurrenceStart;
I64 firstOccurenceOf_(String s, U8 ch, OccurrenceStart occurrence);
#define firstOccurenceOf(string, ch, ...)                                      \
    firstOccurenceOf_(string, ch, (OccurrenceStart){.from = 0, __VA_ARGS__});

I64 lastOccurenceOf(String s, U8 ch);

#endif
