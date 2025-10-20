#ifndef EFI_ACPI_CONFIGURATION_TABLE_H
#define EFI_ACPI_CONFIGURATION_TABLE_H

#include "shared/uuid.h"
typedef struct {
    UUID vendorGUID;
    void *vendorTable;
} ConfigurationTable;

#endif
