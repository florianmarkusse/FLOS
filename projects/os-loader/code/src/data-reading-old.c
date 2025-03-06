#include "abstraction/memory/physical/allocation.h"
#include "os-loader/data-reading.h"

#include "abstraction/log.h"
#include "abstraction/memory/manipulation.h"
#include "efi-to-kernel/generated/kernel-magic.h"
#include "efi-to-kernel/memory/descriptor.h"
#include "efi/error.h"
#include "efi/firmware/base.h"               // for ERROR, Handle
#include "efi/firmware/block-io.h"           // for BLOCK_IO_PROTOCOL...
#include "efi/firmware/file.h"               // for FileProtocol, FIL...
#include "efi/firmware/loaded-image.h"       // for LOADED_IMAGE_PROT...
#include "efi/firmware/simple-file-system.h" // for SIMPLE_FILE_SYSTE...
#include "efi/firmware/system.h"             // for OPEN_PROTOCOL_BY_...
#include "efi/globals.h"                     // for globals
#include "efi/memory.h"
#include "shared/log.h"
#include "shared/macros.h"
#include "shared/maths/maths.h"

string readDiskLbasFromCurrentGlobalImage(Lba diskLba, USize bytes) {
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

    // Loop through and get Block IO protocol for input media ID, for entire
    // disk
    //   NOTE: This assumes the first Block IO found with logical partition
    //   is the entire disk
    BlockIoProtocol *biop;
    USize num_handles = 0;
    Handle *handle_buffer = nullptr;

    status = globals.st->boot_services->locate_handle_buffer(
        BY_PROTOCOL, &BLOCK_IO_PROTOCOL_GUID, nullptr, &num_handles,
        &handle_buffer);
    EXIT_WITH_MESSAGE_IF(status) {
        ERROR(STRING("Could not locate any Block IO Protocols.\n"));
    }

    Handle mediaHandle = nullptr;
    bool readBlocks = false;
    string data;

    KFLUSH_AFTER {
        INFO(STRING("Media ID UEFI loaded from: "));
        INFO(imageBiop->Media->MediaId, NEWLINE);
    }

    for (USize i = 0; i < num_handles && mediaHandle == nullptr; i++) {
        status = globals.st->boot_services->open_protocol(
            handle_buffer[i], &BLOCK_IO_PROTOCOL_GUID, (void **)&biop,
            globals.h, nullptr, OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
        EXIT_WITH_MESSAGE_IF(status) {
            ERROR(STRING("Could not Open Block IO protocol on handle\n"));
        }

        U64 alignedBytes = ALIGN_UP_VALUE(bytes, biop->Media->BlockSize);

        if (biop->Media->MediaId == imageBiop->Media->MediaId) {
            U64 pagesForKernel =
                CEILING_DIV_VALUE(alignedBytes, UEFI_PAGE_SIZE);

            PhysicalAddress address = allocate4KiBPages(pagesForKernel);
            status =
                biop->readBlocks(biop, biop->Media->MediaId, diskLba,
                                 /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
                                 alignedBytes, (void *)address);
            if (!(EFI_ERROR(status))) {
                /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
                data = (string){.buf = (U8 *)address, .len = alignedBytes};

                if (!memcmp(KERNEL_MAGIC, data.buf, COUNTOF(KERNEL_MAGIC))) {
                    readBlocks = true;
                }
            }

            if (!readBlocks) {
                freeBumpPages(pagesForKernel);
            }
        }

        // Close open protocol when done
        globals.st->boot_services->close_protocol(
            handle_buffer[i], &BLOCK_IO_PROTOCOL_GUID, globals.h, nullptr);

        if (readBlocks) {
            break;
        }
    }

    globals.st->boot_services->close_protocol(
        lip->device_handle, &BLOCK_IO_PROTOCOL_GUID, globals.h, nullptr);
    globals.st->boot_services->close_protocol(
        globals.h, &LOADED_IMAGE_PROTOCOL_GUID, globals.h, nullptr);

    if (!readBlocks) {
        EXIT_WITH_MESSAGE {
            ERROR(STRING("Could not find Block IO protocol for disk with ID:"));
            ERROR(imageBiop->Media->MediaId, NEWLINE);
        }
    }

    return data;
}

U32 getDiskImageMediaID() {
    Status status;

    // Get media ID for this disk image
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

    U32 mediaID = biop->Media->MediaId;

    globals.st->boot_services->close_protocol(
        lip->device_handle, &BLOCK_IO_PROTOCOL_GUID, globals.h, nullptr);
    globals.st->boot_services->close_protocol(
        globals.h, &LOADED_IMAGE_PROTOCOL_GUID, globals.h, nullptr);

    return mediaID;
}

// typedef enum { BYTE_SIZE = 0, LBA_START = 1 } DataPartitionLayout;
//
// DataPartitionFile getKernelInfo() {
//     // Get loaded image protocol first to grab device handle to use
//     //   simple file system protocol on
//     LoadedImageProtocol *lip = 0;
//     Status status = globals.st->boot_services->open_protocol(
//         globals.h, &LOADED_IMAGE_PROTOCOL_GUID, (void **)&lip, globals.h,
//         nullptr, OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
//     EXIT_WITH_MESSAGE_IF(status) {
//         ERROR(STRING("Could not open Loaded Image Protocol\n"));
//     }
//
//     // Get Simple File System Protocol for device handle for this loaded
//     //   image, to open the root directory for the ESP
//     SimpleFileSystemProtocol *sfsp = nullptr;
//     status = globals.st->boot_services->open_protocol(
//         lip->device_handle, &SIMPLE_FILE_SYSTEM_PROTOCOL_GUID, (void
//         **)&sfsp, globals.h, nullptr, OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
//     EXIT_WITH_MESSAGE_IF(status) {
//         ERROR(STRING("Could not open Simple File System Protocol\n"));
//     }
//
//     FileProtocol *root = nullptr;
//     status = sfsp->openVolume(sfsp, &root);
//     EXIT_WITH_MESSAGE_IF(status) {
//         ERROR(STRING("Could not Open Volume for root directory in ESP\n"));
//     }
//
//     FileProtocol *file = nullptr;
//     status =
//         root->open(root, &file, u"\\EFI\\FLOS\\KERNEL.INF", FILE_MODE_READ,
//         0);
//     EXIT_WITH_MESSAGE_IF(status) { ERROR(STRING("Could not Open File\n")); }
//
//     FileInfo file_info;
//     USize file_info_size = sizeof(file_info);
//     status = file->getInfo(file, &FILE_INFO_ID, &file_info_size, &file_info);
//     EXIT_WITH_MESSAGE_IF(status) { ERROR(STRING("Could not get file
//     info\n")); }
//
//     string dataFile;
//     dataFile.len = file_info.fileSize;
//
//     PhysicalAddress dataFileAddress;
//
//     status = globals.st->boot_services->allocate_pages(
//         ALLOCATE_ANY_PAGES, LOADER_DATA,
//         CEILING_DIV_VALUE(dataFile.len, UEFI_PAGE_SIZE), &dataFileAddress);
//     if (EFI_ERROR(status) || dataFile.len != file_info.fileSize) {
//         EXIT_WITH_MESSAGE {
//             ERROR(STRING("Could not allocate memory for file\n"));
//         }
//     }
//
//     /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
//     dataFile.buf = (U8 *)dataFileAddress;
//
//     status = file->read(file, &dataFile.len, dataFile.buf);
//     if (EFI_ERROR(status) || dataFile.len != file_info.fileSize) {
//         EXIT_WITH_MESSAGE {
//             ERROR(STRING("Could not read file into buffer\n"));
//         }
//     }
//
//     root->close(root);
//     file->close(file);
//
//     globals.st->boot_services->close_protocol(lip->device_handle,
//                                               &SIMPLE_FILE_SYSTEM_PROTOCOL_GUID,
//                                               globals.h, nullptr);
//
//     globals.st->boot_services->close_protocol(
//         globals.h, &LOADED_IMAGE_PROTOCOL_GUID, globals.h, nullptr);
//
//     // Assumes the below file structure:
//     // KERNEL_SIZE_BYTES=132456
//     // KERNEL_START_LBA=123
//     StringIter lines;
//     DataPartitionFile kernelFile;
//     DataPartitionLayout layout = BYTE_SIZE;
//     TOKENIZE_STRING(dataFile, lines, '\n', 0) {
//         StringIter tokens;
//         bool second = false;
//         TOKENIZE_STRING(lines.string, tokens, '=', 0) {
//             if (second) {
//                 switch (layout) {
//                 case BYTE_SIZE: {
//                     U64 bytes = 0;
//                     for (U64 i = 0; i < tokens.string.len; i++) {
//                         bytes = bytes * 10 + (tokens.string.buf[i] - '0');
//                     }
//
//                     kernelFile.bytes = bytes;
//                     break;
//                 }
//                 case LBA_START: {
//                     U64 lbaStart = 0;
//                     for (U64 i = 0; i < tokens.string.len; i++) {
//                         lbaStart = lbaStart * 10 + (tokens.string.buf[i] -
//                         '0');
//                     }
//                     kernelFile.lbaStart = lbaStart;
//                     break;
//                 }
//                 }
//             }
//             second = true;
//         }
//         layout++;
//     }
//
//     return kernelFile;
// }
