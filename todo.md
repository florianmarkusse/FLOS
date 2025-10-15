- ALl the exponents I am using inside non-constant structs can be converted to U8 instead to save memory?
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
- get rid of lots of virtual stuff in policy
  - move some stuff in policy to virtual?
  - Move policy & physical & (most of) virtual to be arch-independent?
- maybe find some way to get a simple post-order tree iterator or something instead of defining it everywhere
- fix abstraction/memory/virtual/converter.h stuff. I think just define externals? Or maybe this is right too idk
- Fix duplicate stuff in memory mappings
- create macro for ctzl clzl stuff, so I dont need to pick the sizes?
- Look into fixing todo's
- policy is quite architecture-dependent. What's required to completely make it so?
- Remove NOLINTNEXTLINE?
- go over abstraction folders and find out if you can change the ifdefs into just function calls
- Look into U8* usage, as it can alias everything? -> switch to void*??
- Fix posix tests in physical memory
- if you ever decide to use unit tests in the kernel, need to create an abstraction for test-framework so it can be used in both posix and freestanding environments
- use countof replacement instead of macro
- small enums
- update function calls?? enum names too
  - from createX to xCreate etc
  - VERY_GOOD_TYPE_RESULT -> TYPE_RESULT_VERY_GOOD
  - basically directory structure kind of subject -> action -> x , makes it easier
- c2y updates: Are constexpr functions possible???
- the definitiosn we add to the c code, e.g., `X86`, can collide with actual source code, create a pattern to fix this somehow

- run tests
  failing because includes... should be in a base layer

- Find better way to get aligned memory from allocPhysicalMemory, not "just" finding the bytes + align as it causes nasty fracturing

- cmake stuff??
  - just rewrite it all?
    or
  - lots of add_project should be add_include instead...

- After rewrite of cmake?
  - Figure out a way to easily build and run for various x86 architectures with qemu...
  - Also do the same for support for avx512 , no support for avx512, may mean turning off -march=native or something
- os-loader can make use of freestanding peripheral of screen, would enable us to log even after exiting boot services

| IST  | sti | usage Kernel design rationale                     | Solution                                                                                                                                                                                 |
| ---- | --- | ------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| IST1 | Yes | (after prologue) Page fault?                      | Push registers, Check if inside guard page (stack-overflow?), Calculate 128 bytes down from RSP (Red-zone), then push all interesting vars to that new stack and call that fault handler |
| IST2 | No  | NMI = critical, no nesting                        | Report & Exit                                                                                                                                                                            |
| IST3 | No  | Double Fault = kernel panic, no nesting           | Report Error that caused it? & Exit                                                                                                                                                      |
| IST4 | No  | Machine Check = critical, no nesting              | Report & Exit                                                                                                                                                                            |
| IST5 | Yes | (after prologue) Normal interrupts, allow nesting | Push registers, Calculate 128 bytes down from RSP (Red-zone), then push all interesting vars to that new stack and call that fault handler                                               |

start: 0x0000000000011000 bytes: 585728 bytes: 0x000000000008F000
start: 0x0000000000700000 bytes: 1048576 bytes: 0x0000000000100000
start: 0x000000000080A000 bytes: 4096 bytes: 0x0000000000001000
start: 0x0000000000900000 bytes: 460324864 bytes: 0x000000001B700000
start: 0x000000000080F000 bytes: 4096 bytes: 0x0000000000001000
start: 0x000000001D400000 bytes: 28180480 bytes: 0x0000000001AE0000
start: 0x000000001EFA1000 bytes: 9756672 bytes: 0x000000000094E000
start: 0x000000001FBFF000 bytes: 3510272 bytes: 0x0000000000359000

---

start: 0x0000000000102058 bytes: 14896 bytes: 0x0000000000003A30
start: 0x0000000000100000 bytes: 8064 bytes: 0x0000000000001F80
start: 0x0000000000100780 bytes: 6144 bytes: 0x0000000000001800
start: 0x0000000000100380 bytes: 1024 bytes: 0x0000000000000400
