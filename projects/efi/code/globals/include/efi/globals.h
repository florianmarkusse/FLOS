#ifndef EFI_GLOBALS_H
#define EFI_GLOBALS_H

#include "efi/firmware/base.h"   // for Handle, SystemTable
#include "efi/firmware/system.h" // for Handle, SystemTable

typedef struct {
    Handle h;
    SystemTable *st;
} Configuration;

extern Configuration globals;

#endif
