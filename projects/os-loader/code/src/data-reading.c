#include "os-loader/data-reading.h"

#include "abstraction/log.h"
#include "abstraction/memory/physical/allocation.h"
#include "efi-to-kernel/generated/kernel-magic.h"
#include "efi-to-kernel/memory/definitions.h"
#include "efi/error.h"
#include "efi/firmware/base.h" // for ERROR, Handle
#include "efi/firmware/block-io.h"
#include "efi/firmware/file.h"               // for FileProtocol, FIL...
#include "efi/firmware/loaded-image.h"       // for LOADED_IMAGE_PROT...
#include "efi/firmware/simple-file-system.h" // for SIMPLE_FILE_SYSTE...
#include "efi/firmware/simple-text-input.h"
#include "efi/firmware/system.h" // for OPEN_PROTOCOL_BY_...
#include "efi/globals.h"         // for globals
#include "efi/memory/definitions.h"
#include "efi/memory/physical.h"
#include "efi/memory/virtual.h"
#include "shared/log.h"
#include "shared/maths/maths.h"
#include "shared/memory/converter.h"

static string fetchBIOPFromKernel(Handle handle, U32 mediaIDFromLoadedImage,
                                  U64 startingLBA, U64 bytes, Arena scratch) {
    string result;
    result.len = 0;
    BlockIoProtocol *biop;

    Status status = globals.st->boot_services->open_protocol(
        handle, &BLOCK_IO_PROTOCOL_GUID, (void **)&biop, globals.h, nullptr,
        OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    EXIT_WITH_MESSAGE_IF(status) {
        ERROR(STRING("Could not Open Block IO protocol on handle\n"));
    }

    KFLUSH_AFTER {
        INFO(STRING("[BIOP] "));
        INFO(STRING("media id: "));
        INFO(biop->media->mediaID);
        INFO(STRING("logical: "));
        INFO(biop->media->logicalPartition);
        INFO(STRING(" blockSize: "));
        INFO(biop->media->blockSize);
        INFO(STRING(" lastBlock: "));
        INFO(biop->media->lastBlock, NEWLINE);
    }

    U64 alignedBytes = ALIGN_UP_VALUE(bytes, biop->media->blockSize);

    U64 *blockAddress =
        (U64 *)NEW(&scratch, U8, alignedBytes, 0, biop->media->blockSize);

    KFLUSH_AFTER {
        INFO(STRING("It will be read first on address "));
        INFO((void *)blockAddress);
    }

    status = biop->readBlocks(biop, biop->media->mediaID, 0,
                              /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
                              alignedBytes, (void *)blockAddress);
    if (!(EFI_ERROR(status)) &&
        !memcmp(KERNEL_MAGIC, (void *)blockAddress, COUNTOF(KERNEL_MAGIC))) {
        U64 alignment = MAX(biop->media->blockSize,
                            convertPreferredPageToAvailablePages(
                                (Pages){.pageSize = KERNEL_CODE_MAX_ALIGNMENT,
                                        .numberOfPages = 1})
                                .pageSize);
        U64 kernelAddress =
            getAlignedPhysicalMemoryWithArena(bytes, alignment, scratch);

        KFLUSH_AFTER {
            INFO(STRING("Copying to "));
            INFO((void *)kernelAddress, NEWLINE);
        }

        memcpy((void *)kernelAddress, blockAddress, bytes);
        result = (string){.buf = (void *)blockAddress, .len = bytes};
    } else {
        if (EFI_ERROR(status)) {
            KFLUSH_AFTER { ERROR(STRING("Reading failed h,mmmm\n")); }
        } else {
            KFLUSH_AFTER { ERROR(STRING("Faild memcmp? \n")); }
        }

        if (status == DEVICE_ERROR) {
            KFLUSH_AFTER { ERROR(STRING("device error....\n")); }
        }
        if (status == NO_MEDIA) {
            KFLUSH_AFTER { ERROR(STRING("no media....\n")); }
        }
        if (status == MEDIA_CHANGED) {
            KFLUSH_AFTER { ERROR(STRING("media changed\n")); }
        }
        if (status == BAD_BUFFER_SIZE) {
            KFLUSH_AFTER { ERROR(STRING("baed buffer size\n")); }
        }
        if (status == BAD_BUFFER_SIZE) {
            KFLUSH_AFTER { ERROR(STRING("invalid parameter\n")); }
        }
    }

    globals.st->boot_services->close_protocol(handle, &BLOCK_IO_PROTOCOL_GUID,
                                              globals.h, nullptr);

    return result;
}

string readDiskLbasFromCurrentLoadedImage(Lba diskLba, USize bytes,
                                          Arena scratch) {
    Status status;

    LoadedImageProtocol *lip = nullptr;
    status = globals.st->boot_services->open_protocol(
        globals.h, &LOADED_IMAGE_PROTOCOL_GUID, (void **)&lip, globals.h,
        nullptr, OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    EXIT_WITH_MESSAGE_IF(status) {
        ERROR(STRING("Could not open Loaded Image Protocol\n"));
    }

    BlockIoProtocol *imageBiop;
    status = globals.st->boot_services->open_protocol(
        lip->device_handle, &BLOCK_IO_PROTOCOL_GUID, (void **)&imageBiop,
        globals.h, nullptr, OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    EXIT_WITH_MESSAGE_IF(status) {
        ERROR(STRING(
            "Could not open Block IO Protocol for this loaded image.\n"));
    }

    USize numberOfHandles = 0;
    Handle *handleBuffer = nullptr;

    status = globals.st->boot_services->locate_handle_buffer(
        BY_PROTOCOL, &BLOCK_IO_PROTOCOL_GUID, nullptr, &numberOfHandles,
        &handleBuffer);
    EXIT_WITH_MESSAGE_IF(status) {
        ERROR(STRING("Could not locate any Block IO Protocols.\n"));
    }

    string data;
    data.len = 0;

    KFLUSH_AFTER {
        ERROR(STRING("Loaded image media id: "));
        ERROR(imageBiop->media->mediaID, NEWLINE);
    }

    for (U64 i = 0; data.len == 0 && i < numberOfHandles; i++) {
        InputKey key;
        while (globals.st->con_in->read_key_stroke(globals.st->con_in, &key) !=
               SUCCESS) {
            ;
        }
        globals.st->con_in->reset(globals.st->con_in, true);
        EXIT_WITH_MESSAGE_IF(status) {
            ERROR(STRING("Could not reset con in.\n"));
        }
        data = fetchBIOPFromKernel(handleBuffer[i], imageBiop->media->mediaID,
                                   diskLba, bytes, scratch);
    }
    if (data.len == 0) {
        EXIT_WITH_MESSAGE {
            ERROR(STRING("Could not load kernel!\n"));
            ERROR(STRING("Number of handles: "));
            ERROR(numberOfHandles, NEWLINE);
        }
    } else {
        KFLUSH_AFTER {
            ERROR(STRING("Loaded kernel\n"));
            ERROR(STRING("location: "));
            ERROR(data.buf, NEWLINE);
            ERROR(STRING("len: "));
            ERROR(data.len, NEWLINE);
        }

        InputKey key;
        while (globals.st->con_in->read_key_stroke(globals.st->con_in, &key) !=
               SUCCESS) {
            ;
        }
        globals.st->con_in->reset(globals.st->con_in, true);
    }

    status = globals.st->boot_services->free_pool(handleBuffer);
    EXIT_WITH_MESSAGE_IF(status) {
        ERROR(STRING("Could not free handle buffer.\n"));
    }
    globals.st->boot_services->close_protocol(
        lip->device_handle, &BLOCK_IO_PROTOCOL_GUID, globals.h, nullptr);
    EXIT_WITH_MESSAGE_IF(status) {
        ERROR(STRING("Could not close lip block io protocol.\n"));
    }
    globals.st->boot_services->close_protocol(
        globals.h, &LOADED_IMAGE_PROTOCOL_GUID, globals.h, nullptr);
    EXIT_WITH_MESSAGE_IF(status) {
        ERROR(STRING("Could not close lip protocol.\n"));
    }

    KFLUSH_AFTER { ERROR(STRING("After closing protocsl")); }
    InputKey key;
    while (globals.st->con_in->read_key_stroke(globals.st->con_in, &key) !=
           SUCCESS) {
        ;
    }

    return data;
}

U32 getDiskImageMediaID() {
    Status status;

    LoadedImageProtocol *lip = nullptr;
    status = globals.st->boot_services->open_protocol(
        globals.h, &LOADED_IMAGE_PROTOCOL_GUID, (void **)&lip, globals.h,
        nullptr, OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    EXIT_WITH_MESSAGE_IF(status) {
        ERROR(STRING("Could not open Loaded Image Protocol\n"));
    }

    BlockIoProtocol *biop;
    status = globals.st->boot_services->open_protocol(
        lip->device_handle, &BLOCK_IO_PROTOCOL_GUID, (void **)&biop, globals.h,
        nullptr, OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    EXIT_WITH_MESSAGE_IF(status) {
        ERROR(STRING(
            "Could not open Block IO Protocol for this loaded image.\n"));
    }

    U32 mediaID = biop->media->mediaID;

    globals.st->boot_services->close_protocol(
        lip->device_handle, &BLOCK_IO_PROTOCOL_GUID, globals.h, nullptr);
    globals.st->boot_services->close_protocol(
        globals.h, &LOADED_IMAGE_PROTOCOL_GUID, globals.h, nullptr);

    return mediaID;
}

typedef enum { BYTE_SIZE = 0, LBA_START = 1 } DataPartitionLayout;

DataPartitionFile getKernelInfo(Arena scratch) {
    LoadedImageProtocol *lip = 0;
    Status status = globals.st->boot_services->open_protocol(
        globals.h, &LOADED_IMAGE_PROTOCOL_GUID, (void **)&lip, globals.h,
        nullptr, OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    EXIT_WITH_MESSAGE_IF(status) {
        ERROR(STRING("Could not open Loaded Image Protocol\n"));
    }

    SimpleFileSystemProtocol *sfsp = nullptr;
    status = globals.st->boot_services->open_protocol(
        lip->device_handle, &SIMPLE_FILE_SYSTEM_PROTOCOL_GUID, (void **)&sfsp,
        globals.h, nullptr, OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    EXIT_WITH_MESSAGE_IF(status) {
        ERROR(STRING("Could not open Simple File System Protocol\n"));
    }

    FileProtocol *root = nullptr;
    status = sfsp->openVolume(sfsp, &root);
    EXIT_WITH_MESSAGE_IF(status) {
        ERROR(STRING("Could not Open Volume for root directory in ESP\n"));
    }

    FileProtocol *file = nullptr;
    status =
        root->open(root, &file, u"\\EFI\\FLOS\\KERNEL.INF", FILE_MODE_READ, 0);
    EXIT_WITH_MESSAGE_IF(status) { ERROR(STRING("Could not Open File\n")); }

    FileInfo file_info;
    USize file_info_size = sizeof(file_info);
    status = file->getInfo(file, &FILE_INFO_ID, &file_info_size, &file_info);
    EXIT_WITH_MESSAGE_IF(status) { ERROR(STRING("Could not get file info\n")); }

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
    // KERNEL_START_LBA=123
    StringIter lines;
    DataPartitionFile kernelFile;
    DataPartitionLayout layout = BYTE_SIZE;
    TOKENIZE_STRING(dataFile, lines, '\n', 0) {
        StringIter tokens;
        bool second = false;
        TOKENIZE_STRING(lines.string, tokens, '=', 0) {
            if (second) {
                switch (layout) {
                case BYTE_SIZE: {
                    U64 bytes = 0;
                    for (U64 i = 0; i < tokens.string.len; i++) {
                        bytes = bytes * 10 + (tokens.string.buf[i] - '0');
                    }

                    kernelFile.bytes = bytes;
                    break;
                }
                case LBA_START: {
                    U64 lbaStart = 0;
                    for (U64 i = 0; i < tokens.string.len; i++) {
                        lbaStart = lbaStart * 10 + (tokens.string.buf[i] - '0');
                    }
                    kernelFile.lbaStart = lbaStart;
                    break;
                }
                }
            }
            second = true;
        }
        layout++;
    }

    return kernelFile;
}
