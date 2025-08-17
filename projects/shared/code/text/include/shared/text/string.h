#ifndef SHARED_TEXT_STRING_H
#define SHARED_TEXT_STRING_H

#include "abstraction/memory/manipulation.h"
#include "shared/assert.h"
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

static inline bool stringEquals(String a, String b) {
    return a.len == b.len && (a.len == 0 || !memcmp(a.buf, b.buf, a.len));
}

static inline String stringCopy(String dest, String src) {
    ASSERT(dest.len >= src.len);

    memcpy(dest.buf, src.buf, src.len);
    dest.len = src.len;
    return dest;
}
static inline U8 getChar(String str, U32 index) {
    ASSERT(index < str.len);

    return str.buf[index];
}

static inline U8 getCharOr(String str, U32 index, I8 or) {
    if (index < 0 || index >= str.len) {
        return (U8) or ;
    }
    return str.buf[index];
}

static inline U8 *getCharPtr(String str, U32 index) {
    ASSERT(index < str.len);

    return &str.buf[index];
}

static inline bool containsChar(String s, U8 ch) {
    for (typeof(s.len) i = 0; i < s.len; i++) {
        if (s.buf[i] == ch) {
            return true;
        }
    }
    return false;
}

static inline String splitString(String s, U8 token, U32 from) {
    ASSERT(from >= 0 && from < s.len);

    for (typeof(s.len) i = from; i < s.len; i++) {
        if (s.buf[i] == token) {
            return (String){.buf = getCharPtr(s, from), .len = i - from};
        }
    }

    return (String){.buf = getCharPtr(s, from), .len = s.len - from};
}

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

static inline I64 firstOccurenceOfFrom(String s, U8 ch, U32 from) {
    ASSERT(from >= 0 && from < s.len);

    for (typeof(s.len) i = from; i < s.len; i++) {
        if (s.buf[i] == ch) {
            return (I64)i;
        }
    }
    return -1;
}
static inline I64 firstOccurenceOf(String s, U8 ch) {
    return firstOccurenceOfFrom(s, ch, 0);
}

static inline I64 lastOccurenceOf(String s, U8 ch) {
    // Is uint here so it will wrap at 0
    for (typeof(s.len) i = s.len - 1; i >= s.len; i--) {
        if (s.buf[i] == ch) {
            return (I64)i;
        }
    }
    return -1;
}

#endif
