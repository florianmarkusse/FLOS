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

`C` is already pulling part of the curtain back:

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

First, I will describe the _copy_ variant of dynamic arrays in a little more
detail. Thereafter follows the different implementation I am describing. After
that, a comparison of both these variants in terms of performance is done on my
host machine on both Linux and my own OS, FLOS. Lastly, I will discuss some
common concerns with my implementation and assuage those to the best of my
abilities.

## The _copy_ Variant

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
if this page of memory is already mapped (specifically, it causes a page fault),
and maps it to _physical_ memory if need be. The resulting code in a
POSIX-compliant system would look something like this:

```c
int TEST_MEMORY_AMOUNT = 1 << 30; // Or some other obscenely large number that will never be reached.
                                  // (Or the maximum bytes you want the array to hold.)
int maxEntries = TEST_MEMORY_AMOUNT / sizeof(int);
int *buffer = mmap(NULL, TEST_MEMORY_AMOUNT, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
int size = 0;
for (int i = 0; i <= 2; i++) {
    if (size == maxEntries) {
        // Do something in case you hit the max entries
    }
    buffer[i] = (i + 1) * 10;
    size++;
}
```

I admit that the `mmap` call looks a bit daunting, but that is merely the
result of the POSIX interface. Gone is the check for growing beyond the array's
original capacity and resizing if need be! The OS now transparently handles
growing this dynamic array for you using [demand
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

### _Virtual_ Memory

An idea more than a half-century old now, [_virtual_
memory](https://en.wikipedia.org/wiki/Virtual_memory) is an abstraction atop
"real" _physical_ memory implemented in all modern CPU architectures, excluding
embedded devices and the like. This abstraction allows programs to no longer
care about the fragmentation of _physical_ memory, or how data even gets into
memory. The program operates in the idealized world of _virtual_ memory that is
near infinite and contiguous.

How does this work? _Virtual_ memory is a very large range of addresses, 256
TiB or 128 PiB on modern x86 CPUs, that by themselves point to nothing. A lot
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

Want to run program D that takes up 12KiB, but can't run as there is no
contiguous region of 12KiB
+--------------------------------------------------------+
| Free (4KiB)      | Program B (4KiB) | Free (8KiB)      |
+--------------------------------------------------------+
```

```
With Virtual Memory
         --------------------------------------------------------------------------
Virtual                | Program B | Program D                       |
         --------------------------------------------------------------------------
                       \ 4KiB      / 4KiB      | 4KiB      | 4KiB
                        \         /            |           |
           /-------------\--------             |           |
          /               \                    |           |
         /                 \                   |           |
         +-----------------------------------------------------------+
Physical | Free (4KiB)      | Program B (4KiB) | Free (8KiB)         |
         +-----------------------------------------------------------+
         0
```

The _virtual_ address space in the example above is split up into pages of 4KiB.
Each of these pages can be mapped separately to a 4KiB page of _physical_
memory. This way, we are now able to accommodate program D to run on our CPU:
the first page is mapped to the free 4KiB _physical_ memory before program B
and the second page is mapped to the free 4KiB _physical_ memory after program
B.

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

A more graphical explanation is shown below.

```
A large buffer of virtual memory, the first part is already mapped as you can see
Virtual  [ 10 ][ 20 ][ 30 ][    ][----][----][----][----][----][----] ... and on
           |      |     |    |
Physical [ 10 ][ 20 ][ 30 ][    ][ 90 ][    ][    ][    ][    ]

Add one more (40):
Virtual  [ 10 ][ 20 ][ 30 ][ 40 ][----][----][----][----][----][----] ... and on
           |     |     |     |
Physical [ 10 ][ 20 ][ 30 ][ 40 ][ 90 ][    ][    ][    ][    ]
The next element will write to unmapped virtual memory.


Add another (50):

First, the OS maps a new virtual page to a physical page. Note that this does
not need to be contiguous with the other backing physical pages
Virtual  [ 10 ][ 20 ][ 30 ][ 40 ][    ][    ][    ][    ][----][----] ... and on
           |     |     |     |      \    \     \     \
           |     |     |     |       \    \     \     \
           |     |     |     |        \    \     \     \
           |     |     |     |         \    \     \     \
           |     |     |     |          \    \     \     \
Physical [ 10 ][ 20 ][ 30 ][ 40 ][ 90 ][    ][    ][    ][    ]

Then, write the element to the array
Virtual  [ 10 ][ 20 ][ 30 ][ 40 ][ 50 ][    ][    ][    ][----][----] ... and on
           |     |     |     |      \    \     \     \
           |     |     |     |       \    \     \     \
           |     |     |     |        \    \     \     \
           |     |     |     |         \    \     \     \
           |     |     |     |          \    \     \     \
Physical [ 10 ][ 20 ][ 30 ][ 40 ][ 90 ][ 50 ][    ][    ][    ]
```

A final note before the performance benchmarks: using _virtual_ memory is not
an optional feature in modern CPUs - it is used whether you want to or not.
Therefore, the original _copy_ variant also does the mapping as described
above, and needlessly adds a copy of all the elements on top because the
_virtual_ memory buffer it allocated initially was too small.

Since _virtual_ memory is so bountiful, I consider it insensible to allocate
such a small buffer if you know that you may need more.

## Performance Benchmarks

Now that both implementations are laid out, it's time to put them to the test
and find out what implementation performs better.

The following tests will be run:

- Baseline test: a test using a static array. In other words, write to an array
  where you know the number of entries you want to write beforehand. This test
  should be the fastest of the bunch: the array will already have the right size
  and be mapped into _physical_ memory.
- _copy_ dynamic array test
- _mappable_ dynamic array test

The cases run are:

- Filling the array up to 1GiB of _physical_ memory used
- Filling the array with a variable number of elements, up to a maximum of 1GiB
  of _physical_ memory used, to simulate a random number of elements being added
  to the array. Exactly the use-case for a dynamic array.

In addition, I will run some of the tests with different _virtual_ page sizes
to see the difference in performance. In [Virtual Memory](#Virtual-Memory), I
used page sizes of 4KiB as an example. On the x86 architecture, there are 3
different sizes you can use: 4KiB, 2MiB, and 1GiB. I will skip 1GiB as I don't
see it having a use-case for dynamic arrays.

The hardware I am running these tests on can be found [here](#Hardware). The
code and the configuration to compile the code can be found in this project; I
picked the most performant settings for these tests.

All these tests are run first with a round of warming-up, after which 32
iterations of the same test are run and averaged. All the data can be found in
[Tabular Data](#Tabular-Data).

### Linux / Ubuntu

<div style="display: flex; justify-content: space-between; max-width: 100%;">
  <img src="ubuntu-1GiB-ms.png" style="width: 49%; height: auto;" />
  <img src="ubuntu-random-ms.png" style="width: 49%; height: auto;" />
</div>

As you can see, the _copy_ variant performs the worst in both cases. The _copy_
variant did not have different runs with different page sizes, as the standard
implementation with `realloc` does not allow one to set it.

The _mappable_ variant performs better than the _copy_ variant, even when
setting the page size to the smallest size of 4 KiB. Interestingly, it comes
very close to the baseline variant's performance when the page size is set to
2MiB. The reduced number of page faults (512 instead of 512 x 512) likely plays
a significant role in this. I suspected a considerable speedup going to 2MiB
pages, and having a comparable speed to the baseline is encouraging.

Below is the same graphs, showing not the number of milliseconds of each run,
but the number of clock cycles that everything took. This data was collected to
compare between Ubuntu and my own OS in the next section.

<div style="display: flex; justify-content: space-between; max-width: 100%;">
  <img src="ubuntu-1GiB-cycles.png" style="width: 49%; height: auto;" />
  <img src="ubuntu-random-cycles.png" style="width: 49%; height: auto;" />
</div>

### FLOS

FLOS is an operating system I am building myself. It supports very little, but
it does support demand paging. The results of running the same tests on FLOS
are shown below. FLOS doesn't have time support, so only the clock cycles are
collected.

Additionally, FLOS supports setting page sizes of any power of 2. This is not
supported current hardware. If a buffer wants to use 8KiB _virtual_ pages, the
buffer is split up into 8KiB pages and FLOS maps 2 4KiB pages when it finds an
unmapped page.

<div style="display: flex; justify-content: space-between; max-width: 100%;">
  <img src="FLOS-1GiB-cycles.png" style="width: 49%; height: auto;" />
  <img src="FLOS-random-cycles.png" style="width: 49%; height: auto;" />
</div>

Here we see the same results as observed in the benchmarks done for Ubuntu. The
_copy_ variant is slower across the board, and gets comparatively worse as the
page size increases.

Interestingly, at a page size of 2MiB, the _mappable_ variant outperforms the
baseline. The baseline was done with 1GiB _virtual_ pages, I suspect that the
support for this may not be as strong as it is for 2Mib and 4KiB pages and it
is tripping the CPU up in some other way.

Additionally, going from a 4KiB _virtual_ page size to an emulated 8KiB
_virtual_ page size greatly speeds the writing up for a marginal cost of at
most 4KiB. The fewer - halved, to be exact - page faults play a great part.
It's not just about the page sizes that the hardware supports - but also about
the number of page faults. It's a shame that Linux does not support this
emulation of page sizes in this manner.

Finally, I'm happy to report that my OS is faster than Ubuntu at performing
these operations! But, credit where credit is due, on using 2MiB _virtual_
pages, Ubuntu is very competitive. Now I only need to add support for a litany
of features that are supported by any Linux distro... :)

## Discussion and Practical Considerations

In my opinion, the _mappable_ variant blows the _copy_ variant out of the
water. The speed of the _mappable_ variant is greater for all cases.
Additionally, as mentioned before, references made to entries in the dynamic
array are not potentially invalidated as in the _copy_ variant. This benefit
removes the updating of references after a resizing of the array.

A complaint lodged against the _mappable_ variant is that it uses a lot of
_virtual_ memory; more memory than the _physical_ memory available. The
consequence is that a program can suddenly run out of _physical_ memory (or
start swapping memory) once the OS tries to map this _virtual_ memory. However,
any "large" memory allocation on a modern OS currently returns _virtual_ memory
and uses demand paging to map the memory. This problem is endemic in any
scenario where there is more _virtual_ memory allotted than available
_physical_ memory. To avoid this, they often mention disallowing giving out more
_virtual_ memory than there is _physical_ memory available, rendering this
_mappable_ variant infeasible among other _virtual_ memory benefits. I find
this solution to be throwing out the baby with the bathwater, however. An OS
knows which process caused the page fault, or any other implicit events
resulting in a memory request that it can not satisfy. Linux does not normally
signal this to the process, it just starts killing applications. Programs can
receive signals of this state of events and then write code with this
possibility in mind.

Moreover, the _mappable_ solution has a minimum size requirement of 4KiB. Many
very small dynamic arrays would explode in memory usage if they all started
using this design. This is very true. In my experience, one rarely runs into
this scenario, but perhaps your experience is different. In that case, one can
consider implementing a "small-array optimization", akin to a [small-string
optimization](https://pvs-studio.com/en/blog/terms/6658/).

Finally, one can not forget the portability concern. As mentioned before, not
all CPUs use _virtual_ memory. If you are working in that domain, the _copy_
variant may be better or you have other heuristics that may do even better!

There are probably other concerns that I have missed. Feel free to let me know!

## Conclusion

I found it fascinating to see how much faster the _mappable_ variant is over
the _copy_ variant. Even more so with the lack of discourse about
reimplementing such a fundamental data structure.

I would suggest everyone to consider changing their dynamic array
implementation to a _mappable_ variant, your code becomes easier to read and
faster to boot!

## Appendix

### Tabular Data

Here you can find the raw results of the data for all the tests for each Operating System.

#### Linux / Ubuntu

##### Baseline Array

| Final Array Size | Page Size | clock cycles | ms  |
| ---------------- | --------- | ------------ | --- |
| 1GiB             | 4KiB      | 571367989    | 163 |
| 1GiB             | 2MiB      | 436341743    | 124 |
| random <= 1GiB   | 4KiB      | 286846880    | 81  |
| random <= 1GiB   | 2MiB      | 208957329    | 59  |

##### _mappable_ Dynamic Array

| Final Array Size | Page Size | clock cycles | ms  |
| ---------------- | --------- | ------------ | --- |
| 1GiB             | 4KiB      | 1186074432   | 339 |
| 1GiB             | 2MiB      | 453931408    | 129 |
| random <= 1GiB   | 4KiB      | 570306529    | 162 |
| random <= 1GiB   | 2MiB      | 219778049    | 62  |

##### _copy_ Dynamic Array

| Final Array Size | clock cycles | ms  |
| ---------------- | ------------ | --- |
| 1GiB             | 1277500222   | 365 |
| random <= 1GiB   | 641601994    | 183 |

#### FLOS

##### Baseline Array

No page size was set here as I just used the identity-mapped memory available
in my OS. This identity-memory is mapped with 1GiB pages.

| Final Array Size | clock cycles |
| ---------------- | ------------ |
| 1GiB             | 412151675    |
| random <= 1GiB   | 197411003    |

##### _mappable_ Dynamic Array

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

##### _copy_ Dynamic Array

| Final Array Size | clock cycles |
| ---------------- | ------------ |
| 1GiB             | 1074941751   |
| random <= 1GiB   | 613948315    |

## Hardware

The relevant hardware used for the benchmarks above are:

- Intel(R) Core(TM) i7-4770K CPU @ 3.50GHz
- 12GiB DDR3 RAM
