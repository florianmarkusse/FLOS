#include "abstraction/log.h"
#include "posix/test-framework/test.h"
#include "shared/log.h"
#include "shared/memory/allocator/arena.h"
#include "shared/memory/sizes.h"
#include "shared/trees/red-black/tests/red-black-basic.h"
#include "shared/trees/red-black/tests/red-black-memory-manager.h"

#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <sys/mman.h>

static constexpr auto MEMORY_CAP = 1 * GiB;

int main() {
    char *begin = mmap(NULL, MEMORY_CAP, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (begin == MAP_FAILED) {
        PFLUSH_AFTER(STDERR) {
            ERROR(STRING("Failed to allocate memory!\n"));
            ERROR(STRING("Error code: "));
            ERROR(errno, NEWLINE);
            ERROR(STRING("Error message: "));
            ERROR(STRING_LEN(strerror(errno), strlen(strerror(errno))),
                  NEWLINE);
        }
        return -1;
    }
    Arena arena =
        (Arena){.curFree = begin, .beg = begin, .end = begin + MEMORY_CAP};
    if (setjmp(arena.jmp_buf)) {
        PFLUSH_AFTER(STDERR) { ERROR(STRING("Ran out of memory!\n")); }
    }

    testSuiteStart(STRING("Red-Black Trees"));

    testBasicRedBlackTrees(arena);
    testMemoryManagerRedBlackTrees(arena);

    return testSuiteFinish();
}
