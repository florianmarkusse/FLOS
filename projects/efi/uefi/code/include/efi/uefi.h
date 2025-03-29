#ifndef EFI_UEFI_H
#define EFI_UEFI_H

#include "shared/types/types.h"
#include "shared/uuid.h"
// Little-endian here so we swap the traditional 0x55 0xAA here
static constexpr U16 BOOT_SIGNATURE = 0xAA55;

static constexpr UUID EFI_SYSTEM_PARTITION_GUID = {
    .timeLo = 0xC12A7328,
    .timeMid = 0xF81F,
    .timeHiAndVer = 0x11D2,
    .clockSeqHiAndRes = 0xBA,
    .clockSeqLo = 0x4B,
    .node = {0x00, 0xA0, 0xC9, 0x3E, 0xC9, 0x3B}};

// My data partition GUID
// Windows/Linux have their own. We could use one of theirs but where's the fun
// in that?
static constexpr UUID FLOS_BASIC_DATA_GUID = {
    .timeLo = 0x5f68a13c,
    .timeMid = 0xcdae,
    .timeHiAndVer = 0x4372,
    .clockSeqHiAndRes = 0x95,
    .clockSeqLo = 0xc7,
    .node = {0xfb, 0xc3, 0x8a, 0x42, 0xff, 0x3e}};

typedef struct {
    U8 bootIndicator;
    U8 startingCHS[3];
    U8 osType;
    U8 endingCHS[3];
    U32 startingLBA;
    U32 sizeLBA;
} __attribute__((packed)) MBRPartition;

// NOTE: We are using the fact that this is 128 bytes in the GPTHeader as it is
// set in the image builder's configuration.
typedef struct {
    UUID partitionTypeGUID;
    UUID uniquePartitionGUID;
    U64 startingLBA;
    U64 endingLBA;
    U64 attributes;
    U16 partitionNameUTF16[36]; // UCS-2 (UTF-16 limited to code points 0x0000 -
                                // 0xFFFF)
} __attribute__((packed)) GPTPartitionEntry;

#endif
