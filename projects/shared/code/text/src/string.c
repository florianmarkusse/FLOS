#include "shared/text/string.h"
#include "abstraction/memory/manipulation.h"
#include "shared/assert.h"

bool stringEquals(String a, String b) {
    return a.len == b.len && (a.len == 0 || !memcmp(a.buf, b.buf, a.len));
}

bool stringContainsChar(String s, U8 ch) {
    for (typeof(s.len) i = 0; i < s.len; i++) {
        if (s.buf[i] == ch) {
            return true;
        }
    }
    return false;
}

String stringSplit(String s, U8 token, U32 from) {
    ASSERT(from >= 0 && from < s.len);

    for (typeof(s.len) i = from; i < s.len; i++) {
        if (s.buf[i] == token) {
            return (String){.buf = &s.buf[from], .len = i - from};
        }
    }

    return (String){.buf = &s.buf[from], .len = s.len - from};
}

I64 stringOccurrenceOfChar_(String s, U8 ch, U32 from) {
    ASSERT(from >= 0 && from < s.len);

    for (typeof(s.len) i = from; i < s.len; i++) {
        if (s.buf[i] == ch) {
            return (I64)i;
        }
    }
    return -1;
}

I64 stringLastOccurrenceOfChar(String s, U8 ch) {
    for (typeof(s.len) i = s.len; i-- > 0;) {
        if (s.buf[i] == ch) {
            return (I64)i;
        }
    }
    return -1;
}
