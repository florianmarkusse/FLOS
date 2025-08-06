#ifndef SHARED_TEXT_STRING_H
#define SHARED_TEXT_STRING_H

#include "abstraction/memory/manipulation.h"
#include "shared/assert.h"
#include "shared/types/array.h"
#include "shared/types/numeric.h"

typedef struct {
    U8 *buf;
    U64 len;
} string;

typedef MAX_LENGTH_ARRAY(string) string_max_a;

static constexpr string EMPTY_STRING = ((string){0, 0});
#define STRING(s) ((string){(U8 *)(s), sizeof(s) - 1})
#define STRING_LEN(s, len) ((string){(U8 *)(s), len})
#define STRING_PTRS(begin, end)                                                \
    ((string){(U8 *)(begin), (U64)((end) - (begin))})

static inline bool stringEquals(string a, string b) {
    return a.len == b.len && (a.len == 0 || !memcmp(a.buf, b.buf, a.len));
}

static inline string stringCopy(string dest, string src) {
    ASSERT(dest.len >= src.len);

    memcpy(dest.buf, src.buf, src.len);
    dest.len = src.len;
    return dest;
}
static inline U8 getChar(string str, U64 index) {
    ASSERT(index < str.len);

    return str.buf[index];
}

static inline U8 getCharOr(string str, U64 index, I8 or) {
    if (index < 0 || index >= str.len) {
        return (U8) or ;
    }
    return str.buf[index];
}

static inline U8 *getCharPtr(string str, U64 index) {
    ASSERT(index < str.len);

    return &str.buf[index];
}

static inline bool containsChar(string s, U8 ch) {
    for (U64 i = 0; i < s.len; i++) {
        if (s.buf[i] == ch) {
            return true;
        }
    }
    return false;
}

static inline string splitString(string s, U8 token, U64 from) {
    ASSERT(from >= 0 && from < s.len);

    for (U64 i = from; i < s.len; i++) {
        if (s.buf[i] == token) {
            return (string){.buf = getCharPtr(s, from), .len = i - from};
        }
    }

    return (string){.buf = getCharPtr(s, from), .len = s.len - from};
}

typedef struct {
    string string;
    U64 pos;
} StringIter;

#define TOKENIZE_STRING(_string, stringIter, token, startingPosition)          \
    for ((stringIter) = (StringIter){.pos = (startingPosition)};               \
         ((stringIter).pos < (_string).len) &&                                 \
         ((stringIter).string = splitString(_string, token, (stringIter).pos), \
         1);                                                                   \
         (stringIter).pos += (stringIter).string.len + 1)

static inline I64 firstOccurenceOfFrom(string s, U8 ch, U64 from) {
    ASSERT(from >= 0 && from < s.len);

    for (U64 i = from; i < s.len; i++) {
        if (s.buf[i] == ch) {
            return (I64)i;
        }
    }
    return -1;
}
static inline I64 firstOccurenceOf(string s, U8 ch) {
    return firstOccurenceOfFrom(s, ch, 0);
}

static inline I64 lastOccurenceOf(string s, U8 ch) {
    // Is uint here so it will wrap at 0
    for (U64 i = s.len - 1; i >= s.len; i--) {
        if (s.buf[i] == ch) {
            return (I64)i;
        }
    }
    return -1;
}

#endif
