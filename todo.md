- make os-loader when adding physical memory descriptors also use freeList if need be.
  - Create function in management
- have allocMappableMemory take into account the pageSize that will be used to map in case of page fault
- Find better way to get aligned memory from allocPhysicalMemory, not "just" finding the bytes + align as it causes nasty fracturing
- Create some type of "external" folder for x86 for things that are to be exported and use by using the abstraction indirection.
  - Which just uses functions that return simple values to use.
- look into sizeof usage and see if we can replace it with typeof
- per-cpu caches??
- look into alignof calls, and see if I can replace it with the variable

  - https://old.reddit.com/r/C_Programming/comments/1i74hii/quick_hash_tables_and_dynamic_arrays_in_c/m8l40fo/

- Decide on how to implement pageSize setting when fetching mappable memory
- do xsaveopt and xrstor in asm, not in C
- Can we make AVAILABLE_PAGE_SIZES_MASK a function instead to return the mask from anabstraction and see if it gets inlined in -03 mode?
- Look into optimizing struct layouts?
- Decide on how to deal with simd/non-simd stuff in interrupt handlers

  - do some benchmarking? Per interrupt??

- RED-ZONE ???

- Mark functions whose return values should be used as such to avoid bugs.
- Extract page mapping code in real/idt.c
- maths has an extra maths folder that is not necessary
- remove getAvailableMemory from .h file, should not be called directly
- Decision to flush cpu cache or invalidate should be done in the architecture it's running on, not common code
- wfunction-prototype
- add some nicer logs to image-builder? i.e. more data about sizes in bytes and LBA, not one or the other.
- rename abstraction/efi to abstraction/efi-to-kernel
- make alignup/value into functions?
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
- Fix posix tests in physical memory
- if you ever decide to use unit tests in the kernel, need to create an abstraction for test-framework so it can be used in both posix and freestanding environments
- c2y updates: Are constexpr functions possible???

CPU features to implement/turn on in x86

- sse
- avx
