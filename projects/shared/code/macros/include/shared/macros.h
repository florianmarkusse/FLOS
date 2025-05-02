#ifndef SHARED_MACROS_H
#define SHARED_MACROS_H

#define PASTE(x, y) x##y
#define PASTE2(x, y) PASTE(x, y)

#define MACRO_VAR(name) PASTE2(name, __LINE__)

#define STR_HELPER(x) #x
#define STRINGIFY(x) STR_HELPER(x)

#define COUNTOF(a) (sizeof(a) / sizeof(*(a)))

#define OFFSETOF(t, d) __builtin_offsetof(t, d)

#endif
