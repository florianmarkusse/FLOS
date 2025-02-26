2 known "bugs"

- I do not filter the efi memory before passing to kernel. Which means it adds its own memory to the set of free memory which would result in bad problemos.
- The virtual memory mapping never actually frees the used pages for virtual memory, even if all the virtual memory that that page maps are already freed.
