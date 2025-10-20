#ifndef EFI_ACPI_RSDT_H
#define EFI_ACPI_RSDT_H

#include "shared/types/numeric.h"

static constexpr auto ACPI_DESCRIPTION_TABLE_SIGNATURE_LEN = 4;

typedef struct __attribute__((packed)) {
    U8 signature[ACPI_DESCRIPTION_TABLE_SIGNATURE_LEN];
    U32 length;
    U8 rev;
    U8 checksum;
    U8 OEMID[6];
    U8 OEMTableID[8];
    U32 OEMRevision;
    U8 creatorID[4];
    U32 creatorRevision;
} CAcpiDescriptionTableHeader;

typedef struct __attribute__((packed)) {
    CAcpiDescriptionTableHeader header;
    void **descriptionHeaders;
} CAcpiSDT;

#endif
