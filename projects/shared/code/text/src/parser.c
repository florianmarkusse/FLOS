#include "shared/text/parser.h"

U32 U32Parse(String value, U8 base) {
    U32 result = 0;
    for (typeof(value.len) i = 0; i < value.len; i++) {
        U8 digit = value.buf[i] - '0';
        result = (result * base) + digit;
    }
    return result;
}

U16 U16Parse(String value, U8 base) {
    U16 result = 0;
    for (typeof(value.len) i = 0; i < value.len; i++) {
        U8 digit = value.buf[i] - '0';
        result = (result * base) + digit;
    }
    return result;
}
