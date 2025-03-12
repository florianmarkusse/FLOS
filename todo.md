- rename memory page sizes to more explicit -- BASE_PAGE -> 4KIB_PAGE etc.
- Remove NOLINTNEXTLINE?
- Fix posix tests in physical memory
- if you ever decide to use unit tests in the kernel, need to create an abstraction for test-framework so it can be used in both posix and freestanding environments

CPU features to implement/turn on in x86

- sse
- avx
