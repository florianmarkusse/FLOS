#include "os-loader/data-reading.h"

#include "abstraction/log.h"
#include "abstraction/memory/physical/allocation.h"
#include "efi-to-kernel/generated/kernel-magic.h"
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
#include "efi/memory/definitions.h"
#include "efi/memory/physical.h"
#include "shared/log.h"
#include "shared/maths/maths.h"
#include "shared/memory/converter.h"

// NOTE: Once my firmware supports a PartitionInformationProtocol, this function
// can be used to check for the right block protocol. Until then, it cannot.
static void checkForPartitionGUID(Handle handle) {
    PartitionInformationProtocol *partitionInfo;
    Status status = globals.st->boot_services->open_protocol(
        handle, &PARTITION_INFO_PROTOCOL_GUID, (void **)&partitionInfo,
        globals.h, nullptr, OPEN_PROTOCOL_GET_PROTOCOL);

    EXIT_WITH_MESSAGE_IF_EFI_ERROR(status) {
        ERROR(STRING("Could not find partition info protocol."));
    }

    KFLUSH_AFTER {
        INFO(STRING("resvision: "));
        INFO(partitionInfo->revision, NEWLINE);
        INFO(STRING("Type: "));
        INFO(partitionInfo->type, NEWLINE);
        INFO(STRING("System: "));
        INFO(partitionInfo->system, NEWLINE);
    }

    if (partitionInfo->type == PARTITION_TYPE_MBR) {
        KFLUSH_AFTER { INFO(STRING("Is MBR")); }
    }

    if (partitionInfo->type == PARTITION_TYPE_GPT) {
        GPTPartitionEntry *header = &partitionInfo->gpt;
        if (UUIDEquals(header->partitionTypeGUID, FLOS_BASIC_DATA_GUID)) {
            KFLUSH_AFTER { INFO(STRING("Found one with my GUID!!!")); }
        }
    }

    EXIT_WITH_MESSAGE {
        ERROR(STRING("ALL BAD!"));
        ERROR(STRING("ALL BAD!"));
        ERROR(STRING("ALL BAD!"));
        ERROR(STRING("ALL BAD!"));
    }
}

// We first read memory into scratch memory before copying it to the actual
// memory location that we want it in. UEFI's readBlocks implementation has a
// bug on my hardware with reading blocks into addresses that are above 4GiB and
// the aligned memory address can end up being at this level. So we add an
// intermediate step in between.
static string fetchKernelThroughBIOP(Handle handle, U64 bytes, Arena scratch) {
    string result;
    result.len = 0;

    BlockIoProtocol *biop;
    Status status = globals.st->boot_services->open_protocol(
        handle, &BLOCK_IO_PROTOCOL_GUID, (void **)&biop, globals.h, nullptr,
        OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    EXIT_WITH_MESSAGE_IF_EFI_ERROR(status) {
        ERROR(STRING("Could not Open Block IO protocol on handle\n"));
    }

    if (biop->media->blockSize > 0 && biop->media->logicalPartition) {
        U64 alignedBytes = ALIGN_UP_VALUE(bytes, biop->media->blockSize);

        U64 *blockAddress =
            (U64 *)NEW(&scratch, U8, alignedBytes, 0, biop->media->blockSize);

        status =
            biop->readBlocks(biop, biop->media->mediaID, 0,
                             /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
                             alignedBytes, (void *)blockAddress);
        if (!(EFI_ERROR(status)) && !memcmp(KERNEL_MAGIC, (void *)blockAddress,
                                            COUNTOF(KERNEL_MAGIC))) {
            U64 kernelAddress =
                allocateKernelStructure(bytes, 0, true, scratch);

            memcpy((void *)kernelAddress, blockAddress, bytes);
            result = (string){.buf = (void *)kernelAddress, .len = bytes};
        }
    }

    globals.st->boot_services->close_protocol(handle, &BLOCK_IO_PROTOCOL_GUID,
                                              globals.h, nullptr);

    return result;
}

string readKernelFromCurrentLoadedImage(U64 bytes, Arena scratch) {
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
        BY_PROTOCOL, &BLOCK_IO_PROTOCOL_GUID, nullptr, &numberOfHandles,
        &handleBuffer);
    EXIT_WITH_MESSAGE_IF_EFI_ERROR(status) {
        ERROR(STRING("Could not locate any Block IO Protocols.\n"));
    }

    string data;
    data.len = 0;

    for (U64 i = 0; data.len == 0 && i < numberOfHandles; i++) {
        data = fetchKernelThroughBIOP(handleBuffer[i], bytes, scratch);
    }
    if (data.len == 0) {
        EXIT_WITH_MESSAGE {
            ERROR(STRING("Could not load kernel from any available block "
                         "protocol!\nNumber of handles: "));
            ERROR(numberOfHandles, NEWLINE);
        }
    }

    status = globals.st->boot_services->free_pool(handleBuffer);
    EXIT_WITH_MESSAGE_IF_EFI_ERROR(status) {
        ERROR(STRING("Could not free handle buffer.\n"));
    }
    globals.st->boot_services->close_protocol(
        lip->device_handle, &BLOCK_IO_PROTOCOL_GUID, globals.h, nullptr);
    EXIT_WITH_MESSAGE_IF_EFI_ERROR(status) {
        ERROR(STRING("Could not close lip block io protocol.\n"));
    }
    globals.st->boot_services->close_protocol(
        globals.h, &LOADED_IMAGE_PROTOCOL_GUID, globals.h, nullptr);
    EXIT_WITH_MESSAGE_IF_EFI_ERROR(status) {
        ERROR(STRING("Could not close lip protocol.\n"));
    }

    return data;
}

U64 getKernelBytes(Arena scratch) {
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

    FileInfo file_info;
    USize file_info_size = sizeof(file_info);
    status = file->getInfo(file, &FILE_INFO_ID, &file_info_size, &file_info);
    EXIT_WITH_MESSAGE_IF_EFI_ERROR(status) {
        ERROR(STRING("Could not get file info\n"));
    }

    string dataFile;
    dataFile.len = file_info.fileSize;
    dataFile.buf = NEW(&scratch, U8, dataFile.len, 0, UEFI_PAGE_SIZE);

    status = file->read(file, &dataFile.len, dataFile.buf);
    if (EFI_ERROR(status) || dataFile.len != file_info.fileSize) {
        EXIT_WITH_MESSAGE {
            ERROR(STRING("Could not read file into buffer\n"));
        }
    }

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
                U64 bytes = 0;
                for (U64 i = 0; i < tokens.string.len; i++) {
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
