Need the following parts

- Memory map
  - Permanent memory allocation
    - Used/Unused parts
  - Temporary memory allocation
  - Kernel Parameter allocation

Map to

- Immediately freeable memory
  - Free memory
  - Unused permanent memory
  - Unused temporary memory
- Temporary memory
- Kernel parameter memory

1. Add freeable array of memory to PMM
2. Free the array that is used to hold the freeable array
3. After kernel parameters has been fully processed: free kernel parameters
