#include "image-builder/configuration.h"

#include <string.h>

#include "abstraction/log.h"
#include "image-builder/partitions/efi.h"
#include "posix/log.h"
#include "shared/log.h"
#include "shared/maths.h"
#include "shared/text/string.h"

Configuration configuration = {.imageName = (U8 *)"FLOS_UEFI_IMAGE.hdd",
                               .LBASizeBytes = 512,
                               .alignmentLBA = 1};

void setConfiguration(U32 efiApplicationSizeBytes, U32 kernelSizeBytes,
                      U32 alignmentSizeBytes) {
    if (alignmentSizeBytes > configuration.LBASizeBytes) {
        configuration.alignmentLBA =
            alignmentSizeBytes / configuration.LBASizeBytes;
    }
    U32 currentLBA = 0;

    // MBR + primary GPT
    configuration.GPTPartitionTableSizeLBA =
        GPT_PARTITION_TABLE_SIZE / configuration.LBASizeBytes;
    currentLBA += SectionsInLBASize.PROTECTIVE_MBR +
                  SectionsInLBASize.GPT_HEADER +
                  configuration.GPTPartitionTableSizeLBA;
    currentLBA = (U32)alignUp(currentLBA, configuration.alignmentLBA);

    // EFI Partition
    configuration.EFISystemPartitionStartLBA = currentLBA;
    U32 unalignedLBA = calculateEFIPartitionSize((U32)CEILING_DIV_VALUE(
        efiApplicationSizeBytes, (U32)configuration.LBASizeBytes));
    configuration.EFISystemPartitionSizeLBA =
        (U32)alignUp(unalignedLBA, configuration.alignmentLBA);
    currentLBA += configuration.EFISystemPartitionSizeLBA;

    // Data Partition
    configuration.dataPartitionStartLBA = currentLBA;
    configuration.dataPartitionSizeLBA = (U32)CEILING_DIV_VALUE(
        kernelSizeBytes, (U32)configuration.LBASizeBytes);
    currentLBA += configuration.dataPartitionSizeLBA;

    // Backup GPT
    currentLBA +=
        configuration.GPTPartitionTableSizeLBA + SectionsInLBASize.GPT_HEADER;
    configuration.totalImageSizeLBA = currentLBA;
    configuration.totalImageSizeBytes =
        configuration.totalImageSizeLBA * configuration.LBASizeBytes;

    PFLUSH_AFTER(STDOUT) {
        INFO(STRING("Configuration\n"));

        INFO(STRING("Image name: "));
        INFO(STRING_LEN(configuration.imageName,
                        (U32)strlen((char *)configuration.imageName)),
             .flags = NEWLINE);

        INFO(STRING("LBA size bytes: "));
        INFO(configuration.LBASizeBytes, .flags = NEWLINE);

        INFO(STRING("Alignment in LBA: "));
        INFO(configuration.alignmentLBA, .flags = NEWLINE);

        INFO(STRING("total image size LBA: "));
        INFO(configuration.totalImageSizeLBA, .flags = NEWLINE);

        INFO(STRING("total image size bytes: "));
        INFO(configuration.totalImageSizeBytes, .flags = NEWLINE);

        INFO(STRING("GPT partition table size LBA: "));
        INFO(configuration.GPTPartitionTableSizeLBA, .flags = NEWLINE);

        INFO(STRING("EFI partition start LBA: "));
        INFO(configuration.EFISystemPartitionStartLBA, .flags = NEWLINE);

        INFO(STRING("EFI partition size LBA: "));
        INFO(configuration.EFISystemPartitionSizeLBA, .flags = NEWLINE);

        INFO(STRING("Data partition start LBA: "));
        INFO(configuration.dataPartitionStartLBA, .flags = NEWLINE);

        INFO(STRING("Data partition size LBA: "));
        INFO(configuration.dataPartitionSizeLBA, .flags = NEWLINE);
    }
}
