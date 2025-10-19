#ifndef ACPI_SIGNATURES_H
#define ACPI_SIGNATURES_H

#include "shared/text/string.h"

// TODO: is this U32?
typedef enum : U32 {
    FIXED_ACPI_DESCRIPTION_TABLE,
    MULTIPLE_APIC_DESCRIPTION_TABLE,
    HIGH_PRECISION_EVENT_TIMER_TABLE,
    PCI_MEMORY_MAPPED_CONFIGURATION_TABLE,
    WINDOWS_ACPI_EMULATED_DEVICES_TABLE,
    // This entry serves as the number of actual entries in the enum as well as
    // an error value
    ERROR_AND_NUM_TABLES
} ACPITable;

[[nodiscard]] ACPITable ACPITablesToEnum(string signature);

#endif
