#include "abstraction/memory/management/policy.h"

#include "abstraction/interrupts.h"
#include "abstraction/memory/virtual/map.h"
#include "shared/maths/maths.h"
#include "shared/memory/converter.h"
#include "shared/memory/management/definitions.h"
#include "shared/types/types.h"
#include "x86/memory/definitions.h"
#include "x86/memory/physical.h"
#include "x86/memory/policy/virtual.h"

void *allocAndMap(U64 bytes) {
    //
    return allocPhysicalMemory(bytes);
}

void *allocContiguousAndMap(U64 bytes) {
    //
    return allocPhysicalMemory(bytes);
}

void freeMapped(Memory memory) {
    //
    freeMemory(memory);
}
