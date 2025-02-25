#!/bin/bash
set -eo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")"
cd ../

x86_64-testos-elf-gdb -ex "target remote localhost:1234" \
    -ex "dashboard -layout source stack threads assembly variables breakpoints memory history !registers !expressions" \
    -ex "file $(find . -type f -regex './projects/kernel/code/build/.*/DEBUG/kernel')"
