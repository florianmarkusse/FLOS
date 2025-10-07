#ifndef EFI_GLOBALS_H
#define EFI_GLOBALS_H

#include "efi/firmware/base.h"   // for Handle, SystemTable
#include "efi/firmware/system.h" // for Handle, SystemTable
#include "shared/memory/allocator/arena.h"

typedef struct {
    Handle h;
    SystemTable *st;
    Arena uefiMemory;
    Arena kernelPermanent;
    // TODO: I think we can get rid of this tbh, only memory mapper size is
    // using it, which can be put in permanent memory, the footprint isnt that
    // big
    Arena kernelTemporary; // Memory that is jettisoned after kernel is up and
                           // running. E.g., the memory used for the kernel
                           // parameters.
} Configuration;

extern Configuration globals;

#endif
