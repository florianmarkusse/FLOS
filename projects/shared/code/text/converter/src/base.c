#include "abstraction/text/converter/base.h"

#include "abstraction/memory/manipulation.h"
#include "shared/assert.h"
#include "shared/macros.h"
#include "shared/text/converter/buffer.h"
#include "shared/text/string.h"
#include "shared/types/array-types.h"

string stringToString(string data) { return data; }

string charToString(char data, U8_a tmp) {
    tmp.buf[0] = (U8)data;
    return (string){.len = 1, .buf = tmp.buf};
}

string charToStringDefault(char data) {
    return charToString(data, stringConverterBuffer);
}

string boolToString(bool data) {
    return (data ? STRING("true") : STRING("false"));
}

static U8 hexString[] = "0123456789ABCDEF";
string ptrToString(void *data, U8_a tmp) {
    tmp.buf[0] = '0';
    tmp.buf[1] = 'x';

    U64 counter = 2;
    U64 u = (U64)data;
    for (U64 i = 2 * sizeof(u); i-- > 0;) {
        tmp.buf[counter] = hexString[(u >> (4 * i)) & 15];
        counter++;
    }

    return (string){.len = counter, .buf = tmp.buf};
}

string ptrToStringDefault(void *data) {
    return ptrToString(data, stringConverterBuffer);
}

string U64ToString(U64 data, U8_a tmp) {
    U8 *end = tmp.buf + tmp.len;
    U8 *beg = end;
    do {
        *--beg = '0' + (U8)(data % 10);
    } while (data /= 10);
    return (STRING_PTRS(beg, end));
}

string U64ToStringDefault(U64 data) {
    return U64ToString(data, stringConverterBuffer);
}

string I64ToString(I64 data, U8_a tmp) {
    U8 *end = tmp.buf + tmp.len;
    U8 *beg = end;
    I64 t = data > 0 ? -data : data;
    do {
        *--beg = '0' - (U8)(t % 10);
    } while (t /= 10);
    if (data < 0) {
        *--beg = '-';
    }
    return STRING_PTRS(beg, end);
}

string I64ToStringDefault(I64 data) {
    return I64ToString(data, stringConverterBuffer);
}

string stringWithMinSize(string data, U8 minSize, U8_a tmp) {
    if (data.len >= minSize) {
        return data;
    }

    memcpy(tmp.buf, data.buf, data.len);
    U32 extraSpace = (U32)(minSize - data.len);
    memset(tmp.buf + data.len, ' ', extraSpace);

    return STRING_LEN(tmp.buf, data.len + extraSpace);
}

string stringWithMinSizeDefault(string data, U8 minSize) {
    return stringWithMinSize(data, minSize, stringConverterBuffer);
}

string noAppend() {
    ASSERT(false);
    return EMPTY_STRING;
}
