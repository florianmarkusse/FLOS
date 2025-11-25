#include <errno.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if !defined(__INT8_TYPE__) || !defined(__UINT8_TYPE__) ||                     \
    !defined(__INT16_TYPE__) || !defined(__UINT16_TYPE__) ||                   \
    !defined(__INT32_TYPE__) || !defined(__UINT32_TYPE__) ||                   \
    !defined(__INT64_TYPE__) || !defined(__UINT64_TYPE__) ||                   \
    (!defined(__INT8_C) && !defined(__INT8_C_SUFFIX__)) ||                     \
    (!defined(__INT16_C) && !defined(__INT16_C_SUFFIX__)) ||                   \
    (!defined(__INT32_C) && !defined(__INT32_C_SUFFIX__)) ||                   \
    (!defined(__INT64_C) && !defined(__INT64_C_SUFFIX__)) ||                   \
    (!defined(__UINT8_C) && !defined(__UINT8_C_SUFFIX__)) ||                   \
    (!defined(__UINT16_C) && !defined(__UINT16_C_SUFFIX__)) ||                 \
    (!defined(__UINT32_C) && !defined(__UINT32_C_SUFFIX__)) ||                 \
    (!defined(__UINT64_C) && !defined(__UINT64_C_SUFFIX__))
#error "Compiler does not provide fixed-size integer macros."
#endif

#define JOIN(_a, _b) JOIN_LITERALS(_a, _b)
#define JOIN_LITERALS(_a, _b) _a##_b

#define I8_C(_v) JOIN(_v, __INT8_C_SUFFIX__)
#define U8_C(_v) JOIN(_v, __UINT8_C_SUFFIX__)
#define I16_C(_v) JOIN(_v, __INT16_C_SUFFIX__)
#define U16_C(_v) JOIN(_v, __UINT16_C_SUFFIX__)
#define I32_C(_v) JOIN(_v, __INT32_C_SUFFIX__)
#define U32_C(_v) JOIN(_v, __UINT32_C_SUFFIX__)
#define I64_C(_v) JOIN(_v, __INT64_C_SUFFIX__)
#define U64_C(_v) JOIN(_v, __UINT64_C_SUFFIX__)

typedef __INT8_TYPE__ I8;
typedef __UINT8_TYPE__ U8;
typedef __INT16_TYPE__ I16;
typedef __UINT16_TYPE__ U16;
typedef __INT32_TYPE__ I32;
typedef __UINT32_TYPE__ U32;
typedef __INT64_TYPE__ I64;
typedef __UINT64_TYPE__ U64;
typedef __INTPTR_TYPE__ ISize;
typedef __UINTPTR_TYPE__ USize;

typedef U64 U64_pow2;
typedef U32 U32_pow2;
typedef U16 U16_pow2;
typedef U8 U8_pow2;

typedef U8 Exponent;

// static constexpr I8 I8_MIN = -128;
// static constexpr I16 I16_MIN = -32767 - 1;
// static constexpr I32 I32_MIN = -2147483647 - 1;
// static constexpr I64 I64_MIN = -9223372036854775807 - 1;
//
// static constexpr I8 I8_MAX = (127);
// static constexpr I16 I16_MAX = (32767);
// static constexpr I32 I32_MAX = (2147483647);
// static constexpr I64 I64_MAX = (9223372036854775807);
//
// static constexpr U8 U8_MAX = 0xFF;
// static constexpr U16 U16_MAX = 0xFFFF;
// static constexpr U32 U32_MAX = 0xFFFFFFFF;
// static constexpr U64 U64_MAX = 0xFFFFFFFFFFFFFFFF;

typedef float F32;
typedef double F64;
/*typedef long double F128;*/

// static constexpr F32 F32_MIN = (-3.402823466e+38F);
// static constexpr F32 F32_MAX = 3.402823466e+38F;
// static constexpr F32 F32_EPSILON = 1.192092896e-07F;
// static constexpr F32 F32_MIN_POS = 1.175494351e-38F;
//
// static constexpr auto F64_MIN = (-1.7976931348623158e+308);
// static constexpr auto F64_MAX = 1.7976931348623158e+308;
// static constexpr auto F64_EPSILON = 2.2204460492503131e-16;
// static constexpr auto F64_MIN_POS = 2.2250738585072014e-308;

#define TYPED_CONSTANT(x, value)                                               \
    _Generic((x),                                                              \
        I8: I8_C(value),                                                       \
        U8: U8_C(value),                                                       \
        I16: I16_C(value),                                                     \
        U16: U16_C(value),                                                     \
        I32: I32_C(value),                                                     \
        U32: U32_C(value),                                                     \
        I64: I64_C(value),                                                     \
        U64: U64_C(value),                                                     \
        default: "unknown")

#define MAX_VALUE(x)                                                           \
    _Generic((x),                                                              \
        I8: I8_MAX,                                                            \
        U8: U8_MAX,                                                            \
        I16: I16_MAX,                                                          \
        U16: U16_MAX,                                                          \
        I32: I32_MAX,                                                          \
        U32: U32_MAX,                                                          \
        I64: I64_MAX,                                                          \
        U64: U64_MAX,                                                          \
        F32: F32_MAX,                                                          \
        F64: F64_MAX,                                                          \
        default: "unknown")

#define MIN_VALUE(x)                                                           \
    _Generic((x),                                                              \
        I8: I8_MIN,                                                            \
        I16: I16_MIN,                                                          \
        I32: I32_MIN,                                                          \
        I64: I64_MIN,                                                          \
        F32: F32_MIN,                                                          \
        F64: F64_MIN,                                                          \
        default: "unknown")

#define EPSILON_VALUE(x)                                                       \
    _Generic((x), F32: F32_EPSILON, F64: F64_EPSILON, default: "unknown")

#define MIN_POS_VALUE(x)                                                       \
    _Generic((x), F32: F32_MIN_POS, F64: F64_MIN_POS, default: "unknown")

// static constexpr auto BITS_PER_BYTE = 8;

typedef struct {
    U8 *buf;
    U32 len;
} String;

#define STRING(s) ((String){(U8 *)(s), sizeof(s) - 1})
#define STRING_LEN(s, len) ((String){(U8 *)(s), len})
#define STRING_PTRS(begin, end)                                                \
    ((String){(U8 *)(begin), (U32)(((U64)(end)) - ((U64)(begin)))})

[[nodiscard]] bool stringEquals(String a, String b);
[[nodiscard]] bool stringContainsChar(String s, U8 ch);
[[nodiscard]] String stringSplit(String s, U8 token, U32 from);

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
    for (typeof(s.len) i = from; i < s.len; i++) {
        if (s.buf[i] == token) {
            return (String){.buf = &s.buf[from], .len = i - from};
        }
    }

    return (String){.buf = &s.buf[from], .len = s.len - from};
}

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

int fileDescriptorOpen(char *path, int flags) {
    int result = open(path, flags);
    if (result < 0) {
        printf("Could not open file %s: %s", path, strerror(errno));
        return -1;
    }
    return result;
}

int commandRun(char *command, char *fdIn, char *fdOut, char *fdErr) {
    int fdInDescriptor = -1;
    if (fdIn) {
        fdInDescriptor = fileDescriptorOpen(fdIn, O_RDONLY);
        if (fdInDescriptor < 0) {
            printf("Could not create file descriptor %s: %s", fdIn,
                   strerror(errno));
        }
    }

    int fdOutDescriptor = -1;
    if (fdOut) {
        fdOutDescriptor = fileDescriptorOpen(fdOut, O_WRONLY);
        if (fdOutDescriptor < 0) {
            printf("Could not create file descriptor %s: %s", fdOut,
                   strerror(errno));
        }
    }

    int fdErrDescriptor = -1;
    if (fdErr) {
        fdErrDescriptor = fileDescriptorOpen(fdErr, O_WRONLY);
        if (fdErrDescriptor < 0) {
            printf("Could not create file descriptor %s: %s", fdErr,
                   strerror(errno));
        }
    }

    pid_t cpid = fork();
    if (cpid < 0) {
        printf("Could not fork child process: %s", strerror(errno));
        return -1;
    }

    if (cpid == 0) {
        if (fdInDescriptor) {
            if (dup2(fdInDescriptor, STDIN_FILENO) < 0) {
                printf("Could not setup stdin for child process: %s",
                       strerror(errno));
                exit(1);
            }
        }

        if (fdOutDescriptor) {
            if (dup2(fdOutDescriptor, STDOUT_FILENO) < 0) {
                printf("Could not setup stdout for child process: %s",
                       strerror(errno));
                exit(1);
            }
        }

        if (fdErrDescriptor) {
            if (dup2(fdErrDescriptor, STDERR_FILENO) < 0) {
                printf("Could not setup stderr for child process: %s",
                       strerror(errno));
                exit(1);
            }
        }

        char *arguments[2];
        arguments[0] = command;
        arguments[1] = nullptr;

        if (execvp(command, arguments) < 0) {
            printf("Could not exec child process for %s: %s", command,
                   strerror(errno));
            exit(1);
        }
        __builtin_unreachable();
    }

    return cpid;
}

static String ROOT_FOLDER = STRING("FLOS");

int main(int argc, char **argv) {
    printf("hello from builder!\n");

    printf("hello from !\n");

    char buffer[PATH_MAX];
    if (getcwd(buffer, sizeof(buffer)) != NULL) {
        printf("Current working dir: %s\n", buffer);
    } else {
        perror("getcwd() error");
        return 1;
    }
    String cwd = {.buf = (unsigned char *)buffer, .len = strlen(buffer)};

    StringIter folders;
    U32 rootFolderLen = 1; // Count root '/'
    bool rootFolderFound = false;
    STRING_TOKENIZE(cwd, folders, '/', 0) {
        printf("dir: %.*s\n", folders.string.len, (char *)folders.string.buf);
        rootFolderLen += folders.string.len;
        if (stringEquals(folders.string, ROOT_FOLDER)) {
            rootFolderFound = true;
            break;
        }
        rootFolderLen++; // Adding the next '/'
    }

    if (!rootFolderFound) {
        printf("Run me inside FLOS!!!\n");
        return 1;
    }
    cwd.len = rootFolderLen;

    printf("Root folder: %.*s\n", cwd.len, (char *)cwd.buf);

    char *command = "mkdir -p code/build && clang-19 code/src/main.c -o "
                    "code/build/output && code/build/output";

    return 0;
}
