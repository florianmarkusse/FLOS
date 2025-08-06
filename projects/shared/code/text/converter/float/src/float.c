#include "abstraction/text/converter/float.h"

#include "abstraction/memory/manipulation.h"
#include "abstraction/text/converter/base.h"
#include "shared/text/converter/buffer.h"
#include "shared/text/string.h"
#include "shared/types/array-types.h"

String F64ToString(F64 data, U8_a tmp) {
    U64 tmpLen = 0;
    U32 prec = 1000000; // i.e. 6 decimals

    if (data < 0) {
        tmp.buf[tmpLen++] = '-';
        data = -data;
    }

    data += 0.5 / ((F64)prec);      // round last decimal
    if (data >= (F64)(-1UL >> 1)) { // out of long range?
        tmp.buf[tmpLen++] = 'i';
        tmp.buf[tmpLen++] = 'n';
        tmp.buf[tmpLen++] = 'f';
        return STRING_LEN(tmp.buf, tmpLen);
    }

    U64 integral = (U64)data;
    U64 fractional = (U64)((data - (F64)integral) * (F64)prec);

    U8 buf2[64];
    U8_a tmp2 = (U8_a){.buf = buf2, .len = 64};

    String part = U64ToString(integral, tmp2);
    memcpy(tmp.buf + tmpLen, part.buf, part.len);
    tmpLen += part.len;

    tmp.buf[tmpLen++] = '.';

    U8 counter = 0;
    for (U32 i = prec / 10; i > 1; i /= 10) {
        if (i > fractional) {
            counter++;
        }
    }
    memset(tmp.buf + tmpLen, '0', counter);

    part = U64ToString(fractional, tmp2);
    memcpy(tmp.buf + tmpLen, part.buf, part.len);
    tmpLen += part.len;

    return STRING_LEN(tmp.buf, tmpLen);
}

String F64ToStringDefault(F64 data) {
    return F64ToString(data, stringConverterBuffer);
}
