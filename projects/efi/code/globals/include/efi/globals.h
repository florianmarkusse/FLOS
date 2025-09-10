#ifndef EFI_GLOBALS_H
#define EFI_GLOBALS_H

#include "efi/firmware/base.h"   // for Handle, SystemTable
#include "efi/firmware/system.h" // for Handle, SystemTable
#include "shared/memory/allocator/arena.h"

typedef struct {
    Handle h;
    SystemTable *st;
    Arena kernelPermanent;
    Arena kernelTemporary; // Memory that is jettisoned after kernel is up and
                           // running, .e.g., the memory used for the kernel
                           // parameters.
} Configuration;

extern Configuration globals;

#endif
