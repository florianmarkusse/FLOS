- PMM & VMM use same idea
  - Red black tree with free list
  - able to grow very large with dynamic array
- wfunction-prototype
- rename abstraction/efi to abstraction/efi-to-kernel
- make alignup/value into functions?
- look into alignof calls, and see if I can replace it with the variable
- get rid of lots of virtual stuff in policy
  - move some stuff in policy to virtual?
  - Move policy & physical & (most of) virtual to be arch-independent?
- fix abstraction/memory/virtual/converter.h stuff. I think just define externals? Or maybe this is right too idk
- Fix duplicate stuff in memory mappings
- create macro for ctzl clzl stuff, so I dont need to pick the sizes?
- Figure out a way to do better with masks and bit shifts for virtual values
- policy is quite architecture-dependent. What's required to completely make it so?
- Reploe X_VALUE with X_EXP when it is not compile-time known value? I.e. pageSizes array should be bitshifts I think, check GODBOLT!!
- Remove NOLINTNEXTLINE?
- Fix posix tests in physical memory
- if you ever decide to use unit tests in the kernel, need to create an abstraction for test-framework so it can be used in both posix and freestanding environments

CPU features to implement/turn on in x86

- sse
- avx

2883584
