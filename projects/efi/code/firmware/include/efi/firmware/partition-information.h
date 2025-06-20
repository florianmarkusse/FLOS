#ifndef EFI_FIRMWARE_PARTITION_INFORMATION_H
#define EFI_FIRMWARE_PARTITION_INFORMATION_H

#include "efi/uefi.h"
#include "shared/uuid.h"

static constexpr auto PARTITION_INFO_PROTOCOL_GUID =
    (UUID){.ms1 = 0x8cf2f62c,
           .ms2 = 0xbc9b,
           .ms3 = 0x4821,
           .ms4 = {0x80, 0x8d, 0xec, 0x9e, 0xc4, 0x21, 0xa1, 0xa0}};

typedef enum : U32 {
    PARTITION_INFO_PROTOCOL_REVISION = 0x0001000,
} PartitionInfoProtocolRevision;

typedef enum : U32 {
    PARTITION_TYPE_OTHER = 0,
    PARTITION_TYPE_MBR = 1,
    PARTITION_TYPE_GPT = 2,
} PartitionType;

typedef struct __attribute__((packed)) {
    PartitionInfoProtocolRevision revision;
    PartitionType type;
    U8 system;
    U8 reserved[7];
    union {
        MBRPartition mbr;
        GPTPartitionEntry gpt;
    };
} PartitionInformationProtocol;

#endif
