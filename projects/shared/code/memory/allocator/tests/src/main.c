#include "abstraction/memory/virtual/converter.h"
#include "posix/log.h"
#include "shared/log.h"
#include "shared/maths.h"
#include "shared/memory/allocator/arena.h"
#include "shared/memory/allocator/buddy.h"
#include "shared/memory/allocator/node.h"
#include "shared/memory/allocator/status/buddy.h"
#include "shared/memory/allocator/status/node.h"
#include "shared/memory/management/management.h"
#include "shared/memory/sizes.h"
#include "x86/memory/definitions.h"

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

    Exponent orderCount =
        buddyOrderCountOnLargestPageSize(BUDDY_VIRTUAL_PAGE_SIZE_MAX);
    U32 blocksCapacity = 512;
    U64 *backingBuffer = NEW(&arena, U64, .count = orderCount * blocksCapacity);
    Buddy myBuddy;
    buddyInit(&myBuddy, backingBuffer, blocksCapacity, orderCount);
    if (setjmp(myBuddy.memoryExhausted)) {
        PFLUSH_AFTER(STDOUT) { ERROR(STRING("Buddy is empty!\n")); }
        return 1;
    }
    if (setjmp(myBuddy.backingBufferExhausted)) {
        PFLUSH_AFTER(STDOUT) {
            ERROR(STRING("Backing buffer is exhausted!\n"));
        }
        return 1;
    }

    PFLUSH_AFTER(STDOUT) {
        buddyStatusAppend(&myBuddy);
        INFO(STRING("--------------\n"));
    }

    buddyFree(&myBuddy, (Memory){.start = 540672, .bytes = 507904});

    PFLUSH_AFTER(STDOUT) {
        buddyStatusAppend(&myBuddy);
        INFO(STRING("--------------\n"));
    }

    buddyFree(&myBuddy, (Memory){.start = 528384, .bytes = 12288});

    PFLUSH_AFTER(STDOUT) {
        buddyStatusAppend(&myBuddy);
        INFO(STRING("--------------\n"));
    }

    buddyFree(&myBuddy, (Memory){.start = 528384 + 12288, .bytes = 4096});

    PFLUSH_AFTER(STDOUT) {
        buddyStatusAppend(&myBuddy);
        INFO(STRING("--------------\n"));
    }
}
