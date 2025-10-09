#include "os-loader/data-reading.h"

#include "abstraction/log.h"
#include "abstraction/memory/manipulation.h"
#include "efi-to-kernel/memory/definitions.h"
#include "efi/error.h"
#include "efi/firmware/base.h" // for ERROR, Handle
#include "efi/firmware/block-io.h"
#include "efi/firmware/file.h"         // for FileProtocol, FIL...
#include "efi/firmware/loaded-image.h" // for LOADED_IMAGE_PROT...
#include "efi/firmware/partition-information.h"
#include "efi/firmware/simple-file-system.h" // for SIMPLE_FILE_SYSTE...
#include "efi/firmware/simple-text-input.h"
#include "efi/firmware/system.h" // for OPEN_PROTOCOL_BY_...
#include "efi/globals.h"         // for globals
#include "efi/memory/physical.h"
#include "shared/log.h"
#include "shared/maths.h"
#include "shared/memory/converter.h"
#include "shared/memory/management/definitions.h"

// NOTE: Once my firmware supports a PartitionInformationProtocol, this function
// can be used to check for the right block protocol. Until then, it cannot.
static void kernelLoadWithGUID(Handle handle, U32 bytes, Arena scratch,
                               String *result) {
    PartitionInformationProtocol *partitionInfo;
    Status status = globals.st->boot_services->open_protocol(
        handle, &PARTITION_INFO_PROTOCOL_GUID, (void **)&partitionInfo,
        globals.h, nullptr, OPEN_PROTOCOL_GET_PROTOCOL);
    EXIT_WITH_MESSAGE_IF_EFI_ERROR(status) {
        ERROR(STRING("Could not find partition info protocol."));
    }

    if (partitionInfo->type == PARTITION_TYPE_GPT) {
        GPTPartitionEntry *header = &partitionInfo->gpt;
        if (UUIDEquals(header->partitionTypeGUID, FLOS_BASIC_DATA_GUID)) {
            BlockIoProtocol *biop;
            status = globals.st->boot_services->open_protocol(
                handle, &BLOCK_IO_PROTOCOL_GUID, (void **)&biop, globals.h,
                nullptr, OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
            EXIT_WITH_MESSAGE_IF_EFI_ERROR(status) {
                ERROR(STRING("Could not Open Block IO protocol on handle\n"));
            }

            U32 alignedBytes = (U32)alignUp(bytes, biop->media->blockSize);
            U64 *blockAddress = (U64 *)NEW(&scratch, U8, .count = alignedBytes,
                                           .align = biop->media->blockSize);

            status = biop->readBlocks(biop, biop->media->mediaID, 0,
                                      alignedBytes, (void *)blockAddress);
            EXIT_WITH_MESSAGE_IF_EFI_ERROR(status) {
                ERROR(STRING("Could not read blocks from kernel GPT\n"));
            }

            void *kernelAddress =
                NEW(&globals.kernelPermanent, U8, .count = bytes,
                    .align = pageSizesSmallest());

            memcpy(kernelAddress, blockAddress, bytes);
            *result = (String){.buf = (void *)kernelAddress, .len = bytes};

            status = globals.st->boot_services->close_protocol(
                handle, &BLOCK_IO_PROTOCOL_GUID, globals.h, nullptr);
            EXIT_WITH_MESSAGE_IF_EFI_ERROR(status) {
                ERROR(STRING("Could not close BLOCK_IO_PROTOCOL_GUID.\n"));
            }
        }
    }

    status = globals.st->boot_services->close_protocol(
        handle, &PARTITION_INFO_PROTOCOL_GUID, globals.h, nullptr);
    EXIT_WITH_MESSAGE_IF_EFI_ERROR(status) {
        ERROR(STRING("Could not close PARTITION_INFO_PROTOCOL_GUID.\n"));
    }
}

String readKernelFromCurrentLoadedImage(U32 bytes, Arena scratch) {
    Status status;

    LoadedImageProtocol *lip = nullptr;
    status = globals.st->boot_services->open_protocol(
        globals.h, &LOADED_IMAGE_PROTOCOL_GUID, (void **)&lip, globals.h,
        nullptr, OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    EXIT_WITH_MESSAGE_IF_EFI_ERROR(status) {
        ERROR(STRING("Could not open Loaded Image Protocol\n"));
    }

    USize numberOfHandles = 0;
    Handle *handleBuffer = nullptr;

    status = globals.st->boot_services->locate_handle_buffer(
        BY_PROTOCOL, &PARTITION_INFO_PROTOCOL_GUID, nullptr, &numberOfHandles,
        &handleBuffer);
    EXIT_WITH_MESSAGE_IF_EFI_ERROR(status) {
        ERROR(STRING("Could not locate any PARTITION_INFO_PROTOCOL_GUID.\n"));
    }

    String data;
    data.len = 0;

    for (typeof(numberOfHandles) i = 0; data.len == 0 && i < numberOfHandles;
         i++) {
        kernelLoadWithGUID(handleBuffer[i], bytes, scratch, &data);
    }

    if (data.len == 0) {
        EXIT_WITH_MESSAGE {
            ERROR(STRING("Could not load kernel from any available partition "
                         "protocol!\nNumber of handles: "));
            ERROR(numberOfHandles, .flags = NEWLINE);
        }
    }

    status = globals.st->boot_services->free_pool(handleBuffer);
    EXIT_WITH_MESSAGE_IF_EFI_ERROR(status) {
        ERROR(STRING("Could not free handle buffer.\n"));
    }

    status = globals.st->boot_services->close_protocol(
        globals.h, &LOADED_IMAGE_PROTOCOL_GUID, globals.h, nullptr);
    EXIT_WITH_MESSAGE_IF_EFI_ERROR(status) {
        ERROR(STRING("Could not close lip protocol.\n"));
    }

    return data;
}

U32 getKernelBytes(Arena scratch) {
    LoadedImageProtocol *lip = 0;
    Status status = globals.st->boot_services->open_protocol(
        globals.h, &LOADED_IMAGE_PROTOCOL_GUID, (void **)&lip, globals.h,
        nullptr, OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    EXIT_WITH_MESSAGE_IF_EFI_ERROR(status) {
        ERROR(STRING("Could not open Loaded Image Protocol\n"));
    }

    SimpleFileSystemProtocol *sfsp = nullptr;
    status = globals.st->boot_services->open_protocol(
        lip->device_handle, &SIMPLE_FILE_SYSTEM_PROTOCOL_GUID, (void **)&sfsp,
        globals.h, nullptr, OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    EXIT_WITH_MESSAGE_IF_EFI_ERROR(status) {
        ERROR(STRING("Could not open Simple File System Protocol\n"));
    }

    FileProtocol *root = nullptr;
    status = sfsp->openVolume(sfsp, &root);
    EXIT_WITH_MESSAGE_IF_EFI_ERROR(status) {
        ERROR(STRING("Could not Open Volume for root directory in ESP\n"));
    }

    FileProtocol *file = nullptr;
    status =
        root->open(root, &file, u"\\EFI\\FLOS\\KERNEL.INF", FILE_MODE_READ, 0);
    EXIT_WITH_MESSAGE_IF_EFI_ERROR(status) {
        ERROR(STRING("Could not Open File\n"));
    }

    FileInfo fileInfo;
    U64 fileInfoSize = sizeof(fileInfo);
    status = file->getInfo(file, &FILE_INFO_ID, &fileInfoSize, &fileInfo);
    EXIT_WITH_MESSAGE_IF_EFI_ERROR(status) {
        ERROR(STRING("Could not get file info\n"));
    }

    String dataFile;
    dataFile.buf =
        NEW(&scratch, U8, .count = fileInfo.fileSize, .align = UEFI_PAGE_SIZE);
    U64 bufferLen = fileInfo.fileSize;
    status = file->read(file, &bufferLen, dataFile.buf);
    if (EFI_ERROR(status) || bufferLen != fileInfo.fileSize) {
        EXIT_WITH_MESSAGE {
            ERROR(STRING("Could not read file into buffer\n"));
        }
    }
    dataFile.len = (U32)bufferLen;

    root->close(root);
    file->close(file);

    globals.st->boot_services->close_protocol(lip->device_handle,
                                              &SIMPLE_FILE_SYSTEM_PROTOCOL_GUID,
                                              globals.h, nullptr);

    globals.st->boot_services->close_protocol(
        globals.h, &LOADED_IMAGE_PROTOCOL_GUID, globals.h, nullptr);

    // Assumes the below file structure:
    // KERNEL_SIZE_BYTES=132456
    StringIter lines;
    TOKENIZE_STRING(dataFile, lines, '\n', 0) {
        StringIter tokens;
        bool second = false;
        TOKENIZE_STRING(lines.string, tokens, '=', 0) {
            if (second) {
                U32 bytes = 0;
                for (typeof(tokens.string.len) i = 0; i < tokens.string.len;
                     i++) {
                    bytes = bytes * 10 + (tokens.string.buf[i] - '0');
                }

                return bytes;
            } else {
                second = true;
            }
        }
    }

    EXIT_WITH_MESSAGE {
        ERROR(STRING("kernel.inf file was not in expected format\n"));
    }

    __builtin_unreachable();
}
