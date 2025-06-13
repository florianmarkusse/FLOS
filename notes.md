# Test Results

## Linux

```
Starting full mapping test...
Page Size: 4096
Average clockcycles: 1193802249
Average time ms: 341
Page Size: 2097152
Average clockcycles: 323959325
Average time ms: 92


Starting partial mapping test...
Page Size: 4096
Average clockcycles: 568762013
Average time ms: 162
Page Size: 2097152
Average clockcycles: 155327138
Average time ms: 44


Starting full realloc writing test...
Average clockcycles: 1302560093
Average time ms: 372
Starting partial realloc writing test...
Average clockcycles: 640800190
Average time ms: 182
```

# For 2MiB test

```sh
sudo sysctl -w vm.nr_hugepages=512
```

# References

- Perfect growth ratio: https://www.youtube.com/watch?v=GZPqDvG615k
  - Comment from `@fernandogiongo`
- Overcommitting to stand in: https://stackoverflow.com/questions/67363078/reserving-a-large-virtual-memory-space-for-a-dynamic-array-and-mapping-core-memo
- describing the technique in game dev: https://www.gamedeveloper.com/programming/virtual-memory-tricks
- reddit thread: https://www.reddit.com/r/C_Programming/comments/l9iiiw/is_it_a_good_idea_to_allocate_many_gigabytes_for
- Arena allocators: https://www.bytesbeneath.com/p/the-arena-custom-memory-allocators , https://nullprogram.com/blog/2023/09/27/

The worry about overcommit is that processes can run out out of virtual memory. Howeer, that has nothing to do with virtual memory. One can imagine that a process is given x physical memory and can use as much virtual memory as it wants and when it runs out, some signal can be given to the program. Blaming virtual memory for the fact that modern OSes pretend that we have infinite physical memory seems misguided.
