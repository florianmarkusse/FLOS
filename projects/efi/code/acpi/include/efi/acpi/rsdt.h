#ifndef EFI_ACPI_RSDT_H
#define EFI_ACPI_RSDT_H

#include "shared/types/numeric.h"

static constexpr auto ACPI_DESCRIPTION_TABLE_SIGNATURE_LEN = 4;

typedef struct __attribute__((packed)) {
    U8 signature[ACPI_DESCRIPTION_TABLE_SIGNATURE_LEN];
    U32 length;
    U8 rev;
    U8 checksum;
    U8 oem_id[6];
    U8 oem_table_id[8];
    U32 oem_rev;
    U8 creator_id[4];
    U32 creator_rev;
} CAcpiDescriptionTableHeader;

typedef struct __attribute__((packed)) {
    CAcpiDescriptionTableHeader header;
    void **descriptionHeaders;
} CAcpiSDT;

#endif
