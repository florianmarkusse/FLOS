#include "posix/log.h"
#include "shared/log.h"
#include "shared/memory/allocator/arena.h"
#include "shared/memory/allocator/buddy.h"
#include "shared/memory/sizes.h"

#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <sys/mman.h>

static constexpr auto MEMORY_CAP = 1 * GiB;

int main() {
    U8 *begin = mmap(NULL, MEMORY_CAP, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (begin == MAP_FAILED) {
        PFLUSH_AFTER(STDERR) {
            ERROR(STRING("Failed to allocate memory!\n"));
            ERROR(STRING("Error code: "));
            ERROR(errno, .flags = NEWLINE);
            ERROR(STRING("Error message: "));
            ERROR(STRING_LEN(strerror(errno), (U32)strlen(strerror(errno))),
                  .flags = NEWLINE);
        }
        return -1;
    }
    Arena arena =
        (Arena){.curFree = begin, .beg = begin, .end = begin + MEMORY_CAP};
    if (setjmp(arena.jmpBuf)) {
        PFLUSH_AFTER(STDERR) { ERROR(STRING("Ran out of memory!\n")); }
    }

    PFLUSH_AFTER(STDOUT) { INFO(STRING("hello trehrethr\n")); }

    Buddy *myBuddy = buddyInit((1ULL << 34ULL) - 4654, &arena);

    PFLUSH_AFTER(STDOUT) {
        INFO(STRING("addresss: "));
        INFO((void *)myBuddy, .flags = NEWLINE);
    }
}
