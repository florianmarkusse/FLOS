#ifndef X86_MEMORY_DEFINITIONS_H
#define X86_MEMORY_DEFINITIONS_H

#include "shared/types/numeric.h"

static constexpr auto LOWER_HALF_END = 0x0000800000000000;
static constexpr auto HIGHER_HALF_START = 0xFFFF800000000000;

static constexpr struct {
    U64 ENTRIES;
} PageTableFormat = {.ENTRIES = (1ULL << 9ULL)};

static constexpr auto X86_4KIB_PAGE = (1ULL << (12 + (9 * 0)));
static constexpr auto X86_2MIB_PAGE = (1ULL << (12 + (9 * 1)));
static constexpr auto X86_1GIB_PAGE = (1ULL << (12 + (9 * 2)));
// NOTE: Does not really exist in the architecture this OS is targeting. More
// used as a range an entry covers.
static constexpr auto X86_512GIB_PAGE = (1ULL << (12 + (9 * 3)));
static constexpr auto X86_256TIB_PAGE = (1ULL << (12 + (9 * 4)));

static constexpr auto MAX_PAGING_LEVELS = 4;

// NOTE: the size of an entry in the root page table
static constexpr auto PAGE_ROOT_ENTRY_MAX_SIZE =
    (1ULL << (12 + (9 * (MAX_PAGING_LEVELS - 1))));

static constexpr struct {
    U64 PAGE_PRESENT;         // The page is currently in memory
    U64 PAGE_WRITABLE;        // It’s allowed to write to this page
    U64 PAGE_USER_ACCESSIBLE; // If not set, only kernel mode code can access
                              // this page
    U64 PAGE_WRITE_THROUGH;   // Writes go directly to memory
    U64 PAGE_DISABLE_CACHE;   // No cache is used for this page
    U64 PAGE_ACCESSED;        // The CPU sets this bit when this page is used
    U64 PAGE_DIRTY; // The CPU sets this bit when a write to this page occurs
    U64 PAGE_EXTENDED_SIZE; // Must be 0 in level 4 and 1. Created
                            // Huge/large page in level 3/2
    U64 PAGE_GLOBAL; // Page isn’t flushed from caches on address space switch
                     // (PGE bit of CR4 register must be set)
    U64 PAGE_AVAILABLE_9;  // Can be used freely by the OS
    U64 PAGE_AVAILABLE_10; // Can be used freely by the OS
    U64 PAGE_AVAILABLE_11; // Can be used freely by the OS
    U64 PAGE_AVAILABLE_61; // Can be used freely by the OS
    U64 PAGE_AVAILABLE_62; // Can be used freely by the OS
    U64 PAGE_NO_EXECUTE;   // Forbid executing code on this page (the NXE bit in
                           // the EFER register must be set)
    U64 FRAME_OR_NEXT_PAGE_TABLE; // To get the physical address or the next
                                  // page table address of a virtual address.
} VirtualPageMasks = {.PAGE_PRESENT = (1ULL << 0),
                      .PAGE_WRITABLE = (1ULL << 1),
                      .PAGE_USER_ACCESSIBLE = (1ULL << 2),
                      .PAGE_WRITE_THROUGH = (1ULL << 3),
                      .PAGE_DISABLE_CACHE = (1ULL << 4),
                      .PAGE_ACCESSED = (1ULL << 5),
                      .PAGE_DIRTY = (1ULL << 6),
                      .PAGE_EXTENDED_SIZE = (1ULL << 7),
                      .PAGE_GLOBAL = (1ULL << 8),
                      .PAGE_AVAILABLE_9 = (1ULL << 9),
                      .PAGE_AVAILABLE_10 = (1ULL << 10),
                      .PAGE_AVAILABLE_11 = (1ULL << 11),
                      .PAGE_AVAILABLE_61 = (1ULL << 61),
                      .PAGE_AVAILABLE_62 = (1ULL << 62),
                      .PAGE_NO_EXECUTE = (1ULL << 63),
                      .FRAME_OR_NEXT_PAGE_TABLE = 0x000FFFFFFFFF000};

typedef struct __attribute__((aligned(X86_4KIB_PAGE))) {
    U64 pages[PageTableFormat.ENTRIES];
} VirtualPageTable;

#endif
