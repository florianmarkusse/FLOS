#ifndef EFI_ACPI_MADT_H
#define EFI_ACPI_MADT_H

#include "efi/acpi/c-acpi-rsdt.h"
#include "shared/types/numeric.h"

typedef struct __attribute__((packed)) {
    CAcpiSDT header;
    U32 localControllerAddress;
    U32 flags;
    I8 madtEntriesStart[];
} CAcpiMADT;

typedef struct __attribute__((packed)) {
    U8 type;
    U8 length;
} CAcpiMADTHeader;

typedef struct __attribute__((packed)) {
    CAcpiMADTHeader header;
    U8 ACPIProcessorUID;
    U8 LAPICID;
    U32 flags;
} CAcpiMADTLAPIC;

typedef struct __attribute__((packed)) {
    CAcpiMADTHeader header;
    U8 reserved[2];
    U32 X2APICID;
    U32 flags;
    U32 ACPIProcessorUID;
} CAcpiMADTX2APIC;

typedef struct __attribute__((packed)) {
    U8 type;
    U8 length;
    U8 APICID;
    U8 reserved;
    U32 address;
    U32 gsib;
} CAcpiMADTIOAPIC;

typedef struct __attribute__((packed)) {
    CAcpiMADTHeader header;
    U8 reserved1[2];
    U32 iface_no;
    U32 acpi_uid;
    U32 flags;
    U32 parking_ver;
    U32 perf_gsiv;
    U64 parking_addr;
    U64 gicc_base_addr;
    U64 gicv_base_addr;
    U64 gich_base_addr;
    U32 vgic_maint_gsiv;
    U64 gicr_base_addr;
    U64 mpidr;
    U8 power_eff_class;
    U8 reserved2;
    U16 spe_overflow_gsiv;
} CAcpiMADTGICC;

#endif
