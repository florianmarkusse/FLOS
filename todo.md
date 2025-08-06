- Look into optimizing struct layouts? pahole
- Maybe also make some sizes smaller, e.g., standard arrays/strings can be U32 instead of U64? Look how the struct gets laid out if you add a string/array struct to another struct where there is extra "slop" in the struct inside the other struct?
- redundant decls warning
- look into divisiona and see where can used shift instead for power of 2 div
- clean up red-black tree mess somehow? This should be possible to abstract!
- how to hide extern variables from showing up everywhere
- look into struct x = {.y = 5} and check if it's actually necessary to zero initialize the rest of the struct, we can avoid this in most cases i think
- add restrict to basically all pointers? look into aliasing again!!!
- warning pointer cast turn on again? Maybe ask c_programming how to fix this better? Or whether this can be ignored?
- Mark functions whose [[nodiscard]] return values should be used as such to avoid bugs.
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
- update function calls?? enum names too
  - from createX to xCreate etc
  - VERY_GOOD_TYPE_RESULT -> TYPE_RESULT_VERY_GOOD
  - basically directory structure kind of subject -> action -> x , makes it easier
- c2y updates: Are constexpr functions possible???

- Find better way to get aligned memory from allocPhysicalMemory, not "just" finding the bytes + align as it causes nasty fracturing

- cmake stuff??
  - just rewrite it all?
    or
  - lots of add_project should be add_include instead...

| IST  | sti | usage Kernel design rationale                     | Solution                                                                                                                                                                                 |
| ---- | --- | ------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| IST1 | Yes | (after prologue) Page fault?                      | Push registers, Check if inside guard page (stack-overflow?), Calculate 128 bytes down from RSP (Red-zone), then push all interesting vars to that new stack and call that fault handler |
| IST2 | No  | NMI = critical, no nesting                        | Report & Exit                                                                                                                                                                            |
| IST3 | No  | Double Fault = kernel panic, no nesting           | Report Error that caused it? & Exit                                                                                                                                                      |
| IST4 | No  | Machine Check = critical, no nesting              | Report & Exit                                                                                                                                                                            |
| IST5 | Yes | (after prologue) Normal interrupts, allow nesting | Push registers, Calculate 128 bytes down from RSP (Red-zone), then push all interesting vars to that new stack and call that fault handler                                               |
