- Ability to build many object files
  - Add the source to any "target"
  - Add the includes to any "target"
- Set compile definitions and have those immediately be visible in the build
  tool under the same "variable"
  - e.g. if I want to compile with DEBUG , this is passed to the compiler and
    autuomatically a flag/variable/anything called DEBUG or similar is also set
    in the build
- First-class iwyu support
  - Supports mapping files per project correctly
