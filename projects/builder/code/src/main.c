#include <errno.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
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

#define ARRAY_MAX_LENGTH(T)                                                    \
    struct {                                                                   \
        T *buf;                                                                \
        U32 len;                                                               \
        U32 cap;                                                               \
    }

typedef ARRAY_MAX_LENGTH(char *) charPtr_max_a;

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

// TODO: remave exit(1) calls!

static constexpr auto FULL_ACCESS_USER_ONLY = 0700;
int fileDescriptorOpen(char *path, int flags) {
    int result = open(path, flags | O_CREAT | O_APPEND, FULL_ACCESS_USER_ONLY);
    if (result < 0) {
        printf("Could not open file %s: %s", path, strerror(errno));
        return -1;
    }
    return result;
}

// TODO: should just be some arena that you add to I guess...
static charPtr_max_a argumentsHolderCreate() {
    static constexpr auto ARGUMENTS_MAX = 256;
    charPtr_max_a arguments = {
        .buf = malloc(ARGUMENTS_MAX * sizeof(typeof(*arguments.buf))),
        .len = 0,
        .cap = ARGUMENTS_MAX};
    return arguments;
}

// TODO: person should really be freeing this...
static void commandTokenize(charPtr_max_a *argumentsCurrent, String command) {
    String commandCopy = {.buf = malloc(command.len), .len = command.len};
    if (!commandCopy.buf) {
        printf("no memoryn...\n");
    }
    memcpy(commandCopy.buf, command.buf, command.len);

    StringIter tokens;
    // TODO: Really just dupilcate this command and use an arena, I don't
    // like changing the input command...
    // NOTE: this fails if there are
    // multiple spaces in the command, do I care?
    STRING_TOKENIZE(commandCopy, tokens, ' ', 0) {
        tokens.string.buf[tokens.string.len] = '\0';
        argumentsCurrent->buf[argumentsCurrent->len] =
            (char *)tokens.string.buf;
        argumentsCurrent->len++;
    }
}

typedef struct {
    int fdIn;
    int fdOut;
    int fdErr;
} FileDescriptors;

static int commandExecute(charPtr_max_a arguments,
                          FileDescriptors childFileDescriptors) {
    printf("running command: \n");
    for (U32 i = 0; i < arguments.len; i++) {
        printf("%s ", arguments.buf[i]);
    }
    printf("\n");

    pid_t cpid = fork();
    if (cpid < 0) {
        printf("Could not fork child process: %s", strerror(errno));
        return -1;
    }

    if (cpid == 0) {
        if (childFileDescriptors.fdIn >= 0) {
            if (dup2(childFileDescriptors.fdIn, STDIN_FILENO) < 0) {
                printf("Could not setup stdin for child process: %s",
                       strerror(errno));
                exit(1);
            }
        }

        if (childFileDescriptors.fdOut >= 0) {
            if (dup2(childFileDescriptors.fdOut, STDOUT_FILENO) < 0) {
                printf("Could not setup stdout for child process: %s",
                       strerror(errno));
                exit(1);
            }
        }

        if (childFileDescriptors.fdErr >= 0) {
            if (dup2(childFileDescriptors.fdErr, STDERR_FILENO) < 0) {
                printf("Could not setup stderr for child process: %s",
                       strerror(errno));
                exit(1);
            }
        }

        arguments.buf[arguments.len] = nullptr;
        arguments.len++;
        if (execvp(arguments.buf[0], arguments.buf)) {
            printf("Could not exec child process for binary %s: %s",
                   arguments.buf[0], strerror(errno));
            exit(1);
        }
        __builtin_unreachable();
    }

    if (childFileDescriptors.fdIn >= 0) {
        if (close(childFileDescriptors.fdIn)) {
            printf("Could not close fdin descriptor: %s\n", strerror(errno));
            exit(1);
        }
    }
    if (childFileDescriptors.fdOut >= 0) {
        if (close(childFileDescriptors.fdOut)) {
            printf("Could not close fdOut Descriptor: %s\n", strerror(errno));
            exit(1);
        }
    }
    if (childFileDescriptors.fdErr >= 0) {
        if (close(childFileDescriptors.fdErr)) {
            printf("Could not close fdErr Descriptor: %s\n", strerror(errno));
            exit(1);
        }
    }

    return cpid;
}

FileDescriptors childFildDescriptorsCreate(char *fdIn, char *fdOut,
                                           char *fdErr) {
    FileDescriptors result = {
        .fdIn = -1,
        .fdOut = -1,
        .fdErr = -1,
    };

    if (fdIn) {
        result.fdIn = fileDescriptorOpen(fdIn, O_RDONLY);
        if (result.fdIn < 0) {
            printf("Could not create file descriptor %s: %s", fdIn,
                   strerror(errno));
        }
    }

    if (fdOut) {
        result.fdOut = fileDescriptorOpen(fdOut, O_WRONLY);
        if (result.fdOut < 0) {
            printf("Could not create file descriptor %s: %s", fdOut,
                   strerror(errno));
        }
    }

    if (fdErr) {
        result.fdErr = fileDescriptorOpen(fdErr, O_WRONLY);
        if (result.fdErr < 0) {
            printf("Could not create file descriptor %s: %s", fdErr,
                   strerror(errno));
        }
    }

    return result;
}

int commandRun(String command, char *fdIn, char *fdOut, char *fdErr) {
    FileDescriptors childFileDescriptors =
        childFildDescriptorsCreate(fdIn, fdOut, fdErr);
    charPtr_max_a argumentsCurrent = argumentsHolderCreate();
    commandTokenize(&argumentsCurrent, command);

    int result = commandExecute(argumentsCurrent, childFileDescriptors);

    return result;
}

typedef struct {
    String compiler;
    String file;
    String flags;
} CCompilationInput;

static String SRC_DIR = STRING("src");
static String ROOT_DIR = STRING("FLOS");

static String dirPrefixFindSingleOccurrence(String path, String dir) {
    StringIter dirs;
    bool dirFound = false;
    U32 dirPrefixLen = 0;
    STRING_TOKENIZE(path, dirs, '/', 0) {
        if (dirFound && !dirPrefixLen) {
            dirPrefixLen = dirs.pos;
        }

        if (stringEquals(dirs.string, dir)) {
            if (dirFound) {
                return (String){0};
            } else {
                dirFound = true;
            }
        }
    }

    return (String){.buf = path.buf, .len = dirPrefixLen};
}

int CCodeCompile(CCompilationInput *input, char *fdIn, char *fdOut,
                 char *fdErr) {
    charPtr_max_a argumentsCurrent = argumentsHolderCreate();

    commandTokenize(&argumentsCurrent, input->compiler);
    commandTokenize(&argumentsCurrent, input->flags);
    commandTokenize(&argumentsCurrent, STRING("-c"));
    commandTokenize(&argumentsCurrent, input->file);

    String srcDir = dirPrefixFindSingleOccurrence(input->file, SRC_DIR);
    if (!srcDir.len) {
        fprintf(
            stderr,
            "could not find a path that contains a single %.*s, given: %.*s\n",
            SRC_DIR.len, SRC_DIR.buf, input->file.len, input->file.buf);
        return 1;
    }

    String afterPrefix = {.buf = input->file.buf + srcDir.len,
                          .len = input->file.len - srcDir.len};
    // NOTE: +2 for the .o
    U32 outputLen = afterPrefix.len + 2;
    String output = {.buf = malloc(outputLen), .len = outputLen};
    memcpy(output.buf, afterPrefix.buf, afterPrefix.len);
    output.buf[output.len - 2] = '.';
    output.buf[output.len - 1] = 'o';

    printf("input:           %.*s\n", input->file.len, (char *)input->file.buf);
    printf("prefix is:       %.*s\n", srcDir.len, srcDir.buf);
    printf("after prefix is: %.*s\n", afterPrefix.len, afterPrefix.buf);
    printf("output is:       %.*s\n", output.len, output.buf);

    exit(1);
    __builtin_unreachable();

    return 1;

    // commandTokenize(&argumentsCurrent, STRING("-o"));
    // commandTokenize(&argumentsCurrent, input->output);
    //
    // FileDescriptors childFileDescriptors =
    //     childFildDescriptorsCreate(fdIn, fdOut, fdErr);
    //
    // // charPtr_max_a arguments = commandTokenize(command);
    //
    // int result = commandExecute(argumentsCurrent, childFileDescriptors);
    //
    // return result;
}

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

    String rootDir = dirPrefixFindSingleOccurrence(
        (String){.buf = (unsigned char *)buffer, .len = strlen(buffer)},
        ROOT_DIR);
    if (!rootDir.len) {
        fprintf(stderr,
                "Run me inside a path that contains a single %.*s, given: %s\n",
                ROOT_DIR.len, ROOT_DIR.buf, buffer);
        return 1;
    }

    rootDir.buf[rootDir.len] = '\0';
    if (chdir((char *)rootDir.buf)) {
        perror("chdir() error");
        return 1;
    }

    printf("Current dir: %.*s\n", rootDir.len, (char *)rootDir.buf);

    int status;
    int first = commandRun(STRING("mkdir -p projects/example/code/build"),
                           nullptr, nullptr, nullptr);
    waitpid(first, &status, 0);

    CCompilationInput cInput = {.compiler = STRING("clang-19"),
                                .file =
                                    STRING("projects/example/code/src/main.c")};
    int cCcmpilation = CCodeCompile(&cInput, nullptr, nullptr, nullptr);
    waitpid(cCcmpilation, &status, 0);
    int childPid = commandRun(STRING("projects/example/code/build/output"),
                              nullptr, nullptr, nullptr);
    waitpid(childPid, &status, 0);

    return 0;
}
