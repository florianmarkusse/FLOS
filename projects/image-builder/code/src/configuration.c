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

static void lbaSizeAppend(U32 lbaSize) {
    INFO(stringWithMinSizeDefault(STRING_CONVERT(lbaSize), 6));
    INFO(STRING(" LBAs / "));
    INFO(stringWithMinSizeDefault(
        STRING_CONVERT(lbaSize * configuration.LBASizeBytes), 10));
    INFO(STRING(" bytes"), .flags = NEWLINE);
}

static void partitionAppend(U32 lbaStart, U32 lbaSize) {
    INFO(stringWithMinSizeDefault(STRING_CONVERT(lbaStart), 6));
    INFO(STRING(" + "));
    lbaSizeAppend(lbaSize);
}

void configurationSet(U32 efiApplicationSizeBytes, U32 kernelSizeBytes,
                      U32_pow2 alignmentSizeBytes) {
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
    U32 unalignedLBA = EFISystemPartitionSize((U32)ceilingDivide(
        efiApplicationSizeBytes, (U32)configuration.LBASizeBytes));
    configuration.EFISystemPartitionSizeLBA =
        (U32)alignUp(unalignedLBA, configuration.alignmentLBA);
    currentLBA += configuration.EFISystemPartitionSizeLBA;

    // Data Partition
    configuration.dataPartitionStartLBA = currentLBA;
    configuration.dataPartitionSizeLBA =
        (U32)ceilingDivide(kernelSizeBytes, (U32)configuration.LBASizeBytes);
    currentLBA += configuration.dataPartitionSizeLBA;

    // Backup GPT
    currentLBA +=
        configuration.GPTPartitionTableSizeLBA + SectionsInLBASize.GPT_HEADER;
    configuration.totalImageSizeLBA = currentLBA;
    configuration.totalImageSizeBytes =
        configuration.totalImageSizeLBA * configuration.LBASizeBytes;

    static constexpr auto TEXT_SIZE_CHARACTERS_MIN = 30;
    PFLUSH_AFTER(STDOUT) {
        INFO(STRING("Configuration\n"));

        INFO(stringWithMinSizeDefault(STRING("Image name: "),
                                      TEXT_SIZE_CHARACTERS_MIN));
        INFO(STRING_LEN(configuration.imageName,
                        (U32)strlen((char *)configuration.imageName)),
             .flags = NEWLINE);

        INFO(stringWithMinSizeDefault(STRING("LBA size: "),
                                      TEXT_SIZE_CHARACTERS_MIN));
        lbaSizeAppend(1);

        INFO(stringWithMinSizeDefault(STRING("Alignment value: "),
                                      TEXT_SIZE_CHARACTERS_MIN));
        lbaSizeAppend(configuration.alignmentLBA);

        INFO(stringWithMinSizeDefault(STRING("total image: "),
                                      TEXT_SIZE_CHARACTERS_MIN));
        lbaSizeAppend(configuration.totalImageSizeLBA);

        INFO(stringWithMinSizeDefault(STRING("GPT partition table: "),
                                      TEXT_SIZE_CHARACTERS_MIN));
        lbaSizeAppend(configuration.GPTPartitionTableSizeLBA);

        INFO(STRING("\n--- Partitions ---\n"));
        INFO(stringWithMinSizeDefault(STRING("EFI partition: "),
                                      TEXT_SIZE_CHARACTERS_MIN));
        partitionAppend(configuration.EFISystemPartitionStartLBA,
                        configuration.EFISystemPartitionSizeLBA);

        INFO(stringWithMinSizeDefault(STRING("Data partition: "),
                                      TEXT_SIZE_CHARACTERS_MIN));
        partitionAppend(configuration.dataPartitionStartLBA,
                        configuration.dataPartitionSizeLBA);
    }
}
