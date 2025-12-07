release

```sh
mkdir -p code/build && clang-19 -std=c23 code/src/main.c -o code/build/output && code/build/output
```

debug

```
mkdir -p code/build && clang-19 -std=c23 -O0 -g3 -fno-omit-frame-pointer -fsanitize=address,undefined code/src/main.c -o code/build/output && code/build/output
```
