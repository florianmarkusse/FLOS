- cmake stuff??
  - just rewrite it all?
    or
  - lots of add_project should be add_include instead...

- the definitiosn we add to the c code, e.g., `X86`, can collide with actual source code, create a pattern to fix this somehow

- After rewrite of cmake?
- run iwyu
- run tests
  failing because includes... should be in a base layer
  - Figure out a way to easily build and run for various x86 architectures with qemu...
  - Also do the same for support for avx512 , no support for avx512, may mean turning off -march=native or something
- os-loader can make use of freestanding peripheral of screen, would enable us to log even after exiting boot services
- test out using gcc for kernel build too
