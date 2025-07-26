- fix weird GDT stuff and IST loading per CPU
- get cache line size from cpu?
- change memoryAllocator Arena to just be an array, should be easier to work with and clearer. No need for arena since it will only "allocate" a single struct
- create abstraction that fault handler calls to map memory in etc.
- Create tree to map memory, unmapped / certain page sizes to use in fault handler
- make os-loader when memory descriptors use dynamic memory to grow descriptor arena if need be
- have allocMappableMemory take into account the pageSize that will be used to map in case of page fault
- Find better way to get aligned memory from allocPhysicalMemory, not "just" finding the bytes + align as it causes nasty fracturing
- no graphics what happens to the GOP??? Error handling in convert kernel memory not working?
- Clean up virtualForKernel in os-loader
  - It aligns data up, and thus that padding can be freed
  - No checks on if we allocated enough virtual memory
  - No freeing of all the leftover memory
- Find long-term solution for number of descriptors/nodes that are possible in a physical/virtual memory manager, not just a hardcoded/calculated number.
- Create some type of "external" folder for x86 for things that are to be exported and use by using the abstraction indirection.
  - Which just uses functions that return simple values to use.
- look into sizeof usage and see if we can replace it with typeof
- per-cpu caches??
- look into alignof calls, and see if I can replace it with the variable?
- look into sizeof calls, and see if I can replace iw with the variable?

  - https://old.reddit.com/r/C_Programming/comments/1i74hii/quick_hash_tables_and_dynamic_arrays_in_c/m8l40fo/

- Decide on how to implement pageSize setting when fetching mappable memory
- do xsaveopt and xrstor in asm, not in C
- Can we make AVAILABLE_PAGE_SIZES_MASK a function instead to return the mask from anabstraction and see if it gets inlined in -03 mode?
- Look into optimizing struct layouts? pahole
- Maybe also make some sizes smaller, e.g., standard arrays/strings can be U32 instead of U64? Look how the struct gets laid out if you add a string/array struct to another struct where there is extra "slop" in the struct inside the other struct?
- Decide on how to deal with simd/non-simd stuff in interrupt handlers

  - do some benchmarking? Per interrupt??

- redundant decls warning
- look into divisiona and see where can used shift instead for power of 2 div
- RED-ZONE ???
- clean up red-black tree mess somehow? This should be possible to abstract!
- how to hide extern variables from showing up everywhere
- look into struct x = {.y = 5} and check if it's actually necessary to zero initialize the rest of the struct, we can avoid this in most cases i think
- add restrict to basically all pointers? look into aliasing again!!!
- warning pointer cast turn on again? Maybe ask c_programming how to fix this better? Or whether this can be ignored?
- Mark functions whose [[nodiscard]] return values should be used as such to avoid bugs.
- Extract page mapping code in real/idt.c
- remove getAvailableMemory from .h file, should not be called directly
- Decision to flush cpu cache or invalidate should be done in the architecture it's running on, not common code
- wfunction-prototype
- look into caching page table / meta data tables anyway and see if it's faster since it avoids zeroing
- add some nicer logs to image-builder? i.e. more data about sizes in bytes and LBA, not one or the other.
- rename abstraction/efi to abstraction/efi-to-kernel
- make alignup/value into functions? Make a lot of maths macros into functions,
  and look into where you are using VALUE and see if you can replace it by
  divides by 2, which are faster
- get rid of lots of virtual stuff in policy
  - move some stuff in policy to virtual?
  - Move policy & physical & (most of) virtual to be arch-independent?
- fix abstraction/memory/virtual/converter.h stuff. I think just define externals? Or maybe this is right too idk
- Fix duplicate stuff in memory mappings
- create macro for ctzl clzl stuff, so I dont need to pick the sizes?
- Look into fixing todo's
- Figure out a way to do better with masks and bit shifts for virtual values
- policy is quite architecture-dependent. What's required to completely make it so?
- Reploe X_VALUE with X_EXP when it is not compile-time known value? I.e. pageSizes array should be bitshifts I think, check GODBOLT!!
- Remove NOLINTNEXTLINE?
- go over abstraction folders and find out if you can change the ifdefs into just function calls
- Fix posix tests in physical memory
- if you ever decide to use unit tests in the kernel, need to create an abstraction for test-framework so it can be used in both posix and freestanding environments
- use countof replacement instead of macro
- small enums
- c2y updates: Are constexpr functions possible???

IST sti usage Kernel design rationale
IST1 No Double Fault = kernel panic, no nesting
IST2 No NMI = critical, no nesting
IST3 No Machine Check = critical, no nesting
IST4 Yes (after prologue) Normal interrupts, allow nesting
