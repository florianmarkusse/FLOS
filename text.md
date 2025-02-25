```
cmake   -S /home/florian/Desktop/homegrown/projects/efi/code \
  -B /home/florian/Desktop/homegrown/projects/efi/code/build/X86/clang-19/lld-link-19/EFI/with-floats/RELEASE \
  -D PROJECT_FOLDER=efi \
  -D CMAKE_C_COMPILER=clang-19 \
  -D CMAKE_LINKER=lld-link-19 \
  -D CMAKE_BUILD_TYPE=RELEASE \
  -D ENVIRONMENT=EFI \
  -D ARCHITECTURE=X86 \
  -D BUILD_OUTPUT_PATH=/home/florian/Desktop/homegrown/projects/efi/code/build/X86/clang-19/lld-link-19/EFI/with-floats/RELEASE \
  -D REPO_ROOT=/home/florian/Desktop/homegrown \
  -D REPO_DEPENDENCIES=/home/florian/Desktop/homegrown/dependencies \
  -D REPO_PROJECTS=/home/florian/Desktop/homegrown/projects \
  -D PROJECT_TARGETS_FILE=/home/florian/Desktop/homegrown/projects/efi/code/build/targets.txt \
  -D FLOAT_OPERATIONS=true \
  -D BUILD=PROJECT \
  --graphviz=/home/florian/Desktop/homegrown/projects/efi/code/output.dot \
  -D CMAKE_C_INCLUDE_WHAT_YOU_USE="include-what-you-use;-w;-Xiwyu;--mapping_file=/home/florian/Desktop/homegrown/projects/facades.imp;" \
```

```
include-what-you-use -Xiwyu --no_default_mappings -Xiwyu --mapping_file=/home/florian/Desktop/homegrown/projects/facades.imp -DEFI -DFLOAT_OPERATIONS -DPROJECT -DRELEASE -DX86 -I/home/florian/Desktop/homegrown/projects/efi/code/log/include -I/home/florian/Desktop/homegrown/projects/efi-to-kernel/code/include -I/home/florian/Desktop/homegrown/projects/shared/code/assert/include -I/home/florian/Desktop/homegrown/projects/shared/code/text/include -I/home/florian/Desktop/homegrown/projects/shared/code/text/converter/include -I/home/florian/Desktop/homegrown/projects/shared/code/text/converter/float/include -I/home/florian/Desktop/homegrown/projects/shared/code/memory/converter/include -I/home/florian/Desktop/homegrown/projects/shared/code/memory/allocator/include -I/home/florian/Desktop/homegrown/projects/shared/code/memory/management/include -I/home/florian/Desktop/homegrown/projects/shared/code/memory/sizes/include -I/home/florian/Desktop/homegrown/projects/shared/code/log/include -I/home/florian/Desktop/homegrown/projects/shared/code/maths/include -I/home/florian/Desktop/homegrown/projects/shared/code/buffer/include -I/home/florian/Desktop/homegrown/projects/shared/code/uuid/include -I/home/florian/Desktop/homegrown/projects/shared/code/hash/include -I/home/florian/Desktop/homegrown/projects/shared/code/dynamic-array/include -I/home/florian/Desktop/homegrown/projects/shared/code/types/include -I/home/florian/Desktop/homegrown/projects/shared/code/macros/include -I/home/florian/Desktop/homegrown/projects/abstraction/log/code/include -I/home/florian/Desktop/homegrown/projects/abstraction/text/converter/code/include -I/home/florian/Desktop/homegrown/projects/abstraction/memory/manipulation/code/include -I/home/florian/Desktop/homegrown/projects/freestanding/code/memory/manipulation/include -I/home/florian/Desktop/homegrown/projects/efi/code/acpi/include -I/home/florian/Desktop/homegrown/projects/efi/code/firmware/include -I/home/florian/Desktop/homegrown/projects/efi/code/globals/include -I/home/florian/Desktop/homegrown/projects/efi/code/error/include -I/home/florian/Desktop/homegrown/projects/efi/code/memory/include -march=native -m64 -Wall -Wextra -Wconversion -Wno-incompatible-pointer-types-discards-qualifiers -Wno-pointer-sign -Wno-sign-conversion -Wdouble-promotion -Wvla -ffreestanding -nostdlib -nostdinc --target=x86_64-unknown-windows -mgeneral-regs-only -mno-stack-arg-probe -O3 -flto --embed-dir=/home/florian/Desktop/homegrown/projects -O3 -DNDEBUG -std=gnu23 -MD -MT log/CMakeFiles/efi-log.dir/src/log.c.o -MF CMakeFiles/efi-log.dir/src/log.c.o.d -o CMakeFiles/efi-log.dir/src/log.c.o /home/florian/Desktop/homegrown/projects/efi/code/log/src/log.c
```
