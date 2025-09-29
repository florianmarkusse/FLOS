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

static constexpr auto NODES_TOTAL_COUNT = 1000;

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
        (void_a){.buf = NEW(&arena, typeof(*myBuddy.data.blocksFree[0]),
                            .count = NODES_TOTAL_COUNT),
                 .len =
                     NODES_TOTAL_COUNT * sizeof(*myBuddy.data.blocksFree[0])},
        (void_a){.buf = NEW(&arena, void *, .count = NODES_TOTAL_COUNT),
                 .len = NODES_TOTAL_COUNT * sizeof(void *)},
        sizeof(*myBuddy.data.blocksFree[0]),
        alignof(*myBuddy.data.blocksFree[0]));

    buddyFree(&myBuddy,
              (Memory){.start = 0x0000000100000000,
                       .bytes = LOWER_HALF_END - 0x0000000100000000},
              &nodeAllocator);

    PFLUSH_AFTER(STDOUT) {
        buddyStatusAppend(&myBuddy);
        INFO(STRING("--------------\n"));
    }

    buddyFree(&myBuddy, (Memory){.start = HIGHER_HALF_START, .bytes = 1044480},
              &nodeAllocator);

    PFLUSH_AFTER(STDOUT) {
        buddyStatusAppend(&myBuddy);
        INFO(STRING("--------------\n"));
    }
}
