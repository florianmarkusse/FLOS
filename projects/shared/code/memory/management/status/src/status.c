#include "shared/memory/policy/status.h"
#include "shared/memory/allocator/buddy.h"
#include "shared/memory/allocator/status/buddy.h"
#include "shared/memory/allocator/status/node.h"
#include "shared/memory/management/page.h"
#include "shared/memory/management/status.h"

#include "abstraction/log.h"
#include "shared/log.h"
#include "shared/memory/management/management.h"
#include "shared/text/string.h"

void physicalMemoryManagerStatusAppend() { buddyStatusAppend(&buddyPhysical); }

void virtualMemoryManagerStatusAppend() { buddyStatusAppend(&buddyVirtual); }

static AvailableMemoryState getAvailableMemory(Buddy *buddy) {
    AvailableMemoryState result = {0};
    for (U8 i = 0; i < buddyOrderCount(buddy); i++) {
        U32 addresses = buddy->data.blocks[i].len;
        result.addresses += addresses;
        result.memory += addresses * buddyBlockSize(buddy, i);
    }
    return result;
}

AvailableMemoryState physicalMemoryAvailableGet() {
    return getAvailableMemory(&buddyPhysical);
}
AvailableMemoryState virtualMemoryAvailableGet() {
    return getAvailableMemory(&buddyVirtual);
}

void memoryAppend(Memory memory) {
    INFO(STRING("["));
    INFO((void *)memory.start);
    INFO(STRING(", "));
    INFO((void *)memory.start + memory.bytes);
    INFO(STRING("]"));
}

void mappingMemoryAppend(U64 virtualAddress, U64 physicalAddress, U64 size) {
    memoryAppend((Memory){.start = virtualAddress, .bytes = size});
    INFO(STRING(" -> "));
    memoryAppend((Memory){.start = physicalAddress, .bytes = size});
    INFO(STRING(" size: "));
    INFO(size, .flags = NEWLINE);
}

void mappingVirtualGuardPageAppend(U64 virtualAddress, U64 size) {
    memoryAppend((Memory){.start = virtualAddress, .bytes = size});
    INFO(STRING(" -> [GUARD PAGE MAPPING, GUARD PAGE MAPPING] size: "));
    INFO(size, .flags = NEWLINE);
}
