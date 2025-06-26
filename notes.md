# Reimplementing Dynamic Arrays

How do you deal with a variable amount of data? You could opt for a static
array with a maximum number of elements; but how do you pick this maximum
number? Too small, and your data won't fit in the static array. And pick too
large, and you're wasting swaths of memory. A dynamic array neatly solves this
concern by relieving the programmer of having to pick this limit and grow in
size as required by the number of elements added to the dynamic array.

Dealing with a variable amount of data in this manner is such a common occurrence
that dynamic arrays are a staple of programming. Many languages offer
abstractions that hide away all the details that have to do with growing the
array when the number of elements added outgrows its original container.

In `Java`, for example:

```java
ArrayList<Integer> dynamicArray = new ArrayList<>();
for (int i = 1; i <= 3; i++) {
    dynamicArray.add(i * 10);
}
```

, Or `JavaScript`:

```javascript
let dynamicArray = [];
for (let i = 1; i <= 3; i++) {
  dynamicArray.push(i * 10);
}
```

`C` is already showing some of the plumbing required:

```c
int size = 0;
int capacity = 4;
int *dynamicArray = malloc(capacity * sizeof(int));
for (int i = 0; i <= 2; i++) {
    if (size == capacity) {
        capacity *= 2;
        // Ignoring the potential error result for brevity's sake.
        int *temp = realloc(dynamicArray, capacity * sizeof(int));
        dynamicArray = temp;
    }

    dynamicArray[size] = i + 1) * 10;
    size++;
}

```

None of this should come as a surprise. This is how all common programming
languages support dynamic arrays and I will dub it the _copy_ variant in this
article. I want to showcase another way of implementing this staple, which is
superior in my opinion:

- It is faster.
- References you make to any elements in the array stay valid. I.e., no copying
  over of memory is done.

A downside is that it is slightly less portable than the original
implementation: it will only run on 64-bit CPUs with an MMU. This is an
acceptable downside for me, and I think many others.

The rest of this article is laid out as follows. First, I will describe the
_copy_ variant of dynamic arrays in a little more detail. Thereafter follows the
different implementation I am describing. After that, a comparison of both
these variants in terms of performance is done on my host machine. Lastly, I
will showcase these same results in my own (very bare-bones) kernel and put it
head-to-head against Ubuntu.

## The _copy_ variant

This variant is so ubiquitous that I only want to highlight how this
implementation resizes when the initial container's size becomes too small to
hold all the elements:

```
Initial capacity = 4
[ 10 ][ 20 ][ 30 ][    ]   <- 3 elements added, 1 slot left

Add one more (40):
[ 10 ][ 20 ][ 30 ][ 40 ]   <- Now full

Add another (50), trigger resize (capacity x 2 => 8), how?
I: Grow the array's capacity by:
    A: Available unused memory behind the array?
        => Claim the unused memory and extend the capacity
        [ 10 ][ 20 ][ 30 ][ 40 ][    ][    ][    ][    ]
    B: No available unused memory behind the array?
        => Allocate a new region of size 8
        [    ][    ][    ][    ][    ][    ][    ][    ]
        => Copy original elements over into a new region
        [ 10 ][ 20 ][ 30 ][ 40 ][    ][    ][    ][    ]
II: Add (50):
    [ 10 ][ 20 ][ 30 ][ 40 ][ 50 ][    ][    ][    ]
```

If you're lucky, you end up with option A, claiming unused memory behind your
array. However, this is unlikely to happen in practice - that region of memory
is rarely unused - and you will suffer the penalty of copying all the elements
over to a new region. Increasingly so, the larger your array gets.

Much effort has gone into finding the perfect growth ratio, I use a growth rate
of 2 in the examples above. The [Dynamic Array Wikipedia
page](https://en.wikipedia.org/wiki/Dynamic_array#Growth_factor) describes what
the mainstream programming languages commonly use for a growth rate. A great
video on the topic is by [Kay
Lack](https://www.youtube.com/watch?v=GZPqDvG615k).

If you want more control over how your memory is allocated and influence the
above scenarios, I can recommend using custom memory allocators. One of the
simpler allocators, yet applicable in the lion's share of problems, is the
arena allocator. Two great posts on this topic are by [Chris
Wellons](https://nullprogram.com/blog/2023/09/27/) and [Dylan
Falconer](https://www.bytesbeneath.com/p/the-arena-custom-memory-allocators).

## The _mappable_ variant

Onto the new implementation. This implementation allocates a huge, say 1GiB,
region of _virtual_ memory upfront. Upon adding a new element, the CPU "checks"
if this page of memory is already mapped, and maps it to _physical_ memory if
need be. The resulting code in an a POSIX-compliant system would look something
like this:

```c
int TEST_MEMORY_AMOUNT = 1 << 30; // Or some other obscenely large number that will never be reached.
                                  // (Or the maximum bytes the array to hold.)
int maxEntries = TEST_MEMORY_AMOUNT / sizeof(int);
int *buffer = mmap(NULL, TEST_MEMORY_AMOUNT, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
int size = 0;
for (int i = 0; i <= 2; i++) {
    if (size == maxEntries) {
        // Do something in case you hit the max
    }
    buffer[i] = (i + 1) * 10;
    size++;
}
```

I admit, the `mmap` call looks a bit daunting, but that is merely the result of
the POSIX interface. Gone is the check for growing beyond the array's original
capacity and resizing if need be! The OS now transparently handles growing this
dynamic array for you using [demand
paging](https://en.wikipedia.org/wiki/Demand_paging) and does this faster than
you copying your contents.

From here on, I'll refer to this approach as the _mappable_ variant going
forward. I am hardly the first to come up with this idea, though material is
hard to find on the internet. I was able to find this idea mentioned, to mixed
reception at best, in these places:

- [@fernandogiongo's comment from the Kay Lack video
  above](https://www.youtube.com/watch?v=GZPqDvG615k)
- [StackOverflow post](https://www.youtube.com/watch?v=GZPqDvG615k)
- [Game Developer
  forum](https://www.gamedeveloper.com/programming/virtual-memory-tricks)
- [Reddit
  post](https://www.reddit.com/r/C_Programming/comments/l9iiiw/is_it_a_good_idea_to_allocate_many_gigabytes_for)

To get everyone on the same page, I will explain _virtual_ memory below. If you
understand this bad joke, feel free to skip the below section.

### Virtual Memory

An idea more than three-quarter centuries old now, [_virtual_
memory](https://en.wikipedia.org/wiki/Virtual_memory) is an abstraction atop
"real" _physical_ memory implemented in all modern CPU architectures, excluding
embedded devices and the like. This abstraction allows programs to no longer
care about the fragmentation of _physical_ memory, or how data even gets into
memory. The program operates in the idealized world of _virtual_ memory that is
near infinite and contiguous.

How does this work? _Virtual_ memory is a very large range of addresses, 256
TiB or 512 PiB on modern x86 CPUs, that by themselves point to nothing. A lot
of addresses that point to nothing don't sound useful, and you're right. The
crux of _virtual_ memory is that you can program to what _physical_ address a
_virtual_ address maps.

To show you how it obviates the need for caring about fragmentation, consider
the example below:

```
3 Programs running without virtual memory
+--------------------------------------------------------+
| Program A (4Kib) | Program B (4Kib) | Program C (8KiB) |
+--------------------------------------------------------+

Sometime later, we can see there is fragmentation of free memory
+--------------------------------------------------------+
| Free (4KiB)      | Program B (4KiB) | Free (8KiB)      |
+--------------------------------------------------------+

Want to run program D that takes up 12KiB, but can't run as there is no contiguous region of 12KiB
+--------------------------------------------------------+
| Free (4KiB)      | Program B (4KiB) | Free (8KiB)      |
+--------------------------------------------------------+
```

```
With Virtual Memory
         +-----------------------------------------------------------+
Virtual                | Program B | Program D                    |
         +-----------------------------------------------------------+
                       \ 4 KiB     / 4KiB          | 4 KiB
                        \         /               /
           /-------------\--------               /
          /               \                     /
         /                 \                   |
         +-----------------------------------------------------------+
Physical | Free (4KiB)      | Program B (4KiB) | Free (8KiB)      |
         +-----------------------------------------------------------+
         0
```

The virtual address space in the example above is split up into pages of 4KiB.
Each of these pages can be mapped separately to a 4KiB page of physical memory.
This way, we are now able to accommodate program D to run on our CPU: the first
page is mapped to the free 4KiB _physical_ memory before program B and the
second page is mapped to the free 4KiB _physical_ memory after program B.

As to how this mapping is implemented, you can read [Philip
Oppermann](https://os.phil-opp.com/paging-introduction/) post. He does an
excellent job of explaining how this is implemented in modern x86 CPUs. Other
CPU architectures handle this in a comparable but not identical manner. If you
want to implement this, consult the architecture's reference manual. (And make
a drawing on a piece of paper. They call it "paging" because you need a page to
draw on to implement it.)

The implementation of this mapping does not bother us here, it's enough to know
about the existence of _virtual_ memory and how it can be mapped to _physical_
memory and that this mapping can be modified in software at any time.

### Mapping The Dynamic Array

We now understand all the pieces required for this implementation:

1. Allocate a large buffer of _virtual_ memory that currently has no mapping
   associated with it.
2. Write to the buffer as you would any static array.
   1. Once the CPU detects that the page, say page `X`, of the _virtual_ memory
      you are writing to has no mapping, an interrupt will be sent to the
      resident OS.
   2. The OS will find a suitable page of _physical_ memory and update the
      mapping to have _virtual_ page `X` map to this _physical_ page it just
      found.

And that's it! You now have a dynamic array that leverages the "modern" CPU
architectures to have the best of both static and dynamic arrays, respectively:

- no copying of elements around which
  - improve performance,
  - and keeps all pointers to elements in the array valid
- arbitrary number of elements in the array so that
  - no swaths of memory are wasted,
  - and a large number of elements in the array is supported

# Test Results

## Linux / Ubuntu

### Baseline Array

| Final Array Size | Page Size | clock cycles | ms  |
| ---------------- | --------- | ------------ | --- |
| 1GiB             | 4KiB      | 561970000    | 160 |
| 1GiB             | 2MiB      | 689760917    | 197 |

### Mappable Dynamic Array

| Final Array Size | Page Size | clock cycles | ms  |
| ---------------- | --------- | ------------ | --- |
| 1GiB             | 4KiB      | 1186074432   | 339 |
| 1GiB             | 2MiB      | 610235931    | 174 |
| random <= 1GiB   | 4KiB      | 560675325    | 160 |
| random <= 1GiB   | 2MiB      | 294586364    | 83  |

### Reallocable Dynamic Array

| Final Array Size | clock cycles | ms  |
| ---------------- | ------------ | --- |
| 1GiB             | 1277500222   | 365 |
| random <= 1GiB   | 641601994    | 183 |

## FLOS

### Baseline Array

| Final Array Size | clock cycles |
| ---------------- | ------------ |
| 1GiB             | 412151675    |

### Mappable Dynamic Array

| Final Array Size | Page Size | clock cycles |
| ---------------- | --------- | ------------ |
| 1GiB             | 4KiB      | 804633890    |
| 1GiB             | 8KiB      | 680136734    |
| 1GiB             | 16KiB     | 618507788    |
| 1GiB             | 32KiB     | 583336356    |
| 1GiB             | 64KiB     | 569517646    |
| 1GiB             | 128KiB    | 562461534    |
| 1GiB             | 256KiB    | 557894614    |
| 1GiB             | 512KiB    | 561249240    |
| 1GiB             | 1MiB      | 566379819    |
| 1GiB             | 2MiB      | 401222490    |
| random <= 1GiB   | 4KiB      | 384460853    |
| random <= 1GiB   | 8KiB      | 320858290    |
| random <= 1GiB   | 16KiB     | 295480659    |
| random <= 1GiB   | 32KiB     | 278870259    |
| random <= 1GiB   | 64KiB     | 269779252    |
| random <= 1GiB   | 128KiB    | 260260630    |
| random <= 1GiB   | 256KiB    | 261239140    |
| random <= 1GiB   | 512KiB    | 268476012    |
| random <= 1GiB   | 1MiB      | 270906293    |
| random <= 1GiB   | 2MiB      | 192855687    |

### Reallocable Dynamic Array

| Final Array Size | clock cycles |
| ---------------- | ------------ |
| 1GiB             | 1074941751   |
| random <= 1GiB   | 613948315    |
