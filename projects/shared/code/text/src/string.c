#include "shared/text/string.h"
#include "abstraction/memory/manipulation.h"
#include "shared/assert.h"

bool stringEquals(String a, String b) {
    return a.len == b.len && (a.len == 0 || !memcmp(a.buf, b.buf, a.len));
}

String stringCopy(String dest, String src) {
    ASSERT(dest.len >= src.len);

    memcpy(dest.buf, src.buf, src.len);
    dest.len = src.len;
    return dest;
}

U8 getChar(String str, U32 index) {
    ASSERT(index < str.len);

    return str.buf[index];
}

U8 getCharOr(String str, U32 index, I8 or) {
    if (index < 0 || index >= str.len) {
        return (U8)or;
    }
    return str.buf[index];
}

U8 *getCharPtr(String str, U32 index) {
    ASSERT(index < str.len);

    return &str.buf[index];
}

bool containsChar(String s, U8 ch) {
    for (typeof(s.len) i = 0; i < s.len; i++) {
        if (s.buf[i] == ch) {
            return true;
        }
    }
    return false;
}

String splitString(String s, U8 token, U32 from) {
    ASSERT(from >= 0 && from < s.len);

    for (typeof(s.len) i = from; i < s.len; i++) {
        if (s.buf[i] == token) {
            return (String){.buf = getCharPtr(s, from), .len = i - from};
        }
    }

    return (String){.buf = getCharPtr(s, from), .len = s.len - from};
}

I64 firstOccurenceOf_(String s, U8 ch, U32 from) {
    ASSERT(from >= 0 && from < s.len);

    for (typeof(s.len) i = from; i < s.len; i++) {
        if (s.buf[i] == ch) {
            return (I64)i;
        }
    }
    return -1;
}

I64 lastOccurenceOf(String s, U8 ch) {
    // NOTE: postdecrement in the guard check here
    for (typeof(s.len) i = s.len; i-- > 0;) {
        if (s.buf[i] == ch) {
            return (I64)i;
        }
    }
    return -1;
}
