#include "abstraction/memory/virtual/converter.h"
#include "posix/log.h"
#include "shared/log.h"
#include "shared/maths.h"
#include "shared/memory/allocator/arena.h"
#include "shared/memory/allocator/buddy.h"
#include "shared/memory/sizes.h"

#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <sys/mman.h>

static constexpr auto MEMORY_CAP = 8 * GiB;

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
        PFLUSH_AFTER(STDOUT) { ERROR(STRING("Ran out of memory!\n")); }
        return 1;
    }

    PFLUSH_AFTER(STDOUT) { INFO(STRING("hello trehrethr\n")); }

    U64 totalSize = 4 * (1ULL << 30ULL) - 465468;
    Buddy *myBuddy =
        buddyInit(totalSize, pageSizesLargest(), pageSizesSmallest(), &arena);

    U64 start = 0;
    U64 end = (1ULL << 20ULL);
    for (U32 i = 0; i < 3; i++) {
        buddyFreeRegionAdd(myBuddy, alignUp(start, pageSizesSmallest()),
                           alignDown(end, pageSizesSmallest()));
        start = end + 4096;
        end = end * 2;
        PFLUSH_AFTER(STDOUT) { buddyStatusAppend(myBuddy); }
    }
}
