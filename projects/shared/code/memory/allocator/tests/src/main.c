#include "abstraction/memory/virtual/converter.h"
#include "posix/log.h"
#include "shared/log.h"
#include "shared/maths.h"
#include "shared/memory/allocator/arena.h"
#include "shared/memory/allocator/buddy.h"
#include "shared/memory/allocator/node.h"
#include "shared/memory/allocator/status/buddy.h"
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

    Buddy myBuddy;
    buddyInit(&myBuddy, 57);
    if (setjmp(myBuddy.jmpBuf)) {
        PFLUSH_AFTER(STDOUT) {
            ERROR(STRING("Buddy is empty or node allocator full!\n"));
        }
        return 1;
    }

    NodeAllocator nodeAllocator;
    nodeAllocatorInit(
        &nodeAllocator,
        (void_a){.buf =
                     NEW(&arena, typeof(*myBuddy.blocksFree[0]), .count = 1000),
                 .len = 1000 * sizeof(*myBuddy.blocksFree[0])},
        (void_a){.buf = NEW(&arena, void *, .count = 1000),
                 .len = 1000 * sizeof(void *)},
        sizeof(*myBuddy.blocksFree[0]), alignof(*myBuddy.blocksFree[0]));

    buddyFreeRegionAdd(&myBuddy, 0x0000000100000000, LOWER_HALF_END,
                       &nodeAllocator);

    PFLUSH_AFTER(STDOUT) {
        buddyStatusAppend(&myBuddy);
        INFO(STRING("--------------\n"));
    }
}
