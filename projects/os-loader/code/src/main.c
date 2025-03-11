#include "abstraction/efi.h"
#include "abstraction/log.h"
#include "abstraction/memory/physical/allocation.h"
#include "abstraction/memory/virtual/map.h"
#include "efi-to-kernel/kernel-parameters.h"  // for KernelParameters
#include "efi-to-kernel/memory/definitions.h" // for STACK_SIZE
#include "efi-to-kernel/memory/descriptor.h"  // for MemoryDescriptor
#include "efi/acpi/rdsp.h"                    // for getRSDP, RSDP...
#include "efi/error.h"
#include "efi/firmware/base.h"               // for PhysicalAddress
#include "efi/firmware/graphics-output.h"    // for GRAPHICS_OUTP...
#include "efi/firmware/simple-text-output.h" // for SimpleTextOut...
#include "efi/firmware/system.h"             // for PhysicalAddress
#include "efi/globals.h"                     // for globals
#include "efi/memory.h"
#include "os-loader/data-reading.h" // for getKernelInfo
#include "os-loader/memory.h"       // for mapMemoryAt
#include "shared/log.h"
#include "shared/maths/maths.h" // for CEILING_DIV_V...
#include "shared/memory/management/definitions.h"
#include "shared/text/string.h" // for CEILING_DIV_V...
#include "shared/types/types.h" // for U64, U32, USize

Status efi_main(Handle handle, SystemTable *systemtable) {
    globals.h = handle;
    globals.st = systemtable;
    globals.st->con_out->reset(globals.st->con_out, false);
    globals.st->con_out->set_attribute(globals.st->con_out,
                                       BACKGROUND_RED | YELLOW);
    initBumpAllocator();

    initArchitecture();

    KFLUSH_AFTER { INFO(STRING("Going to read kernel info\n")); }
    DataPartitionFile kernelFile = getKernelInfo();

    KFLUSH_AFTER {
        INFO(STRING("Going to load kernel\n"));
        INFO(STRING("\tbytes: "));
        INFO(kernelFile.bytes, NEWLINE);
        INFO(STRING("\tlba start: "));
        INFO(kernelFile.lbaStart, NEWLINE);
    }

    string kernelContent = readDiskLbasFromCurrentLoadedImage(
        kernelFile.lbaStart, kernelFile.bytes);

    KFLUSH_AFTER {
        INFO(STRING("Read kernel content, at memory location:"));
        INFO(kernelContent.buf, NEWLINE);
    }

    KFLUSH_AFTER { INFO(STRING("Attempting to map memory now\n")); }

    mapVirtualRegion(KERNEL_CODE_START,
                     (PagedMemory){.start = (U64)kernelContent.buf,
                                   .numberOfPages = CEILING_DIV_VALUE(
                                       kernelContent.len, UEFI_PAGE_SIZE)},
                     UEFI_PAGE_SIZE);

    KFLUSH_AFTER {
        INFO(STRING(
            "Going to collect necessary info, then exit bootservices\n"));
    }
    GraphicsOutputProtocol *gop = nullptr;
    Status status = globals.st->boot_services->locate_protocol(
        &GRAPHICS_OUTPUT_PROTOCOL_GUID, nullptr, (void **)&gop);
    EXIT_WITH_MESSAGE_IF(status) {
        ERROR(STRING("Could not locate locate GOP\n"));
    }

    KFLUSH_AFTER {
        INFO(STRING("The graphics buffer location is at: "));
        INFO((void *)gop->mode->frameBufferBase, NEWLINE);
        INFO(STRING("The graphics buffer size is: "));
        INFO(gop->mode->frameBufferSize, NEWLINE);
    }

    KFLUSH_AFTER { INFO(STRING("Idnetity mapping\n")); }
    identityMapPhysicalMemory(gop->mode->frameBufferBase +
                              gop->mode->frameBufferSize);

    KFLUSH_AFTER { INFO(STRING("Allocating space for stack\n")); }
    PhysicalAddress stackEnd =
        allocAndZero(CEILING_DIV_VALUE(STACK_SIZE, UEFI_PAGE_SIZE));

    mapVirtualRegion(STACK_END,
                     (PagedMemory){.start = stackEnd,
                                   .numberOfPages = CEILING_DIV_VALUE(
                                       STACK_SIZE, UEFI_PAGE_SIZE)},
                     UEFI_PAGE_SIZE);

    KFLUSH_AFTER {
        INFO(STRING("The phyiscal stack will go down from: "));
        INFO((void *)stackEnd + STACK_SIZE, NEWLINE);
        INFO(STRING("to: "));
        INFO((void *)stackEnd, NEWLINE);

        INFO(STRING("The virtual stack will go down from: "));
        INFO((void *)STACK_START, NEWLINE);
        INFO(STRING("to: "));
        INFO((void *)STACK_END, NEWLINE);
    }

    KFLUSH_AFTER { INFO(STRING("Allocating space for kernel parameters\n")); }
    PhysicalAddress kernelParams =
        allocAndZero(CEILING_DIV_VALUE(KERNEL_PARAMS_SIZE, UEFI_PAGE_SIZE));

    mapVirtualRegion(KERNEL_PARAMS_START,
                     (PagedMemory){.start = kernelParams,
                                   .numberOfPages = CEILING_DIV_VALUE(
                                       KERNEL_PARAMS_SIZE, UEFI_PAGE_SIZE)},
                     UEFI_PAGE_SIZE);

    KFLUSH_AFTER {
        INFO(STRING("The phyiscal kernel params goes from: "));
        INFO((void *)kernelParams, NEWLINE);
        INFO(STRING("to: "));
        INFO((void *)kernelParams + KERNEL_PARAMS_SIZE, NEWLINE);

        INFO(STRING("The virtual kernel params goes from: "));
        INFO((void *)KERNEL_PARAMS_START, NEWLINE);
        INFO(STRING("to: "));
        INFO((void *)KERNEL_PARAMS_START + KERNEL_PARAMS_SIZE, NEWLINE);
    }

    /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
    KernelParameters *params = (KernelParameters *)kernelParams;

    params->fb.columns = gop->mode->info->horizontalResolution;
    params->fb.rows = gop->mode->info->verticalResolution;
    params->fb.scanline = gop->mode->info->pixelsPerScanLine;
    params->fb.ptr = gop->mode->frameBufferBase;
    params->fb.size = gop->mode->frameBufferSize;

    RSDPResult rsdp = getRSDP(globals.st->number_of_table_entries,
                              globals.st->configuration_table);
    if (!rsdp.rsdp) {
        EXIT_WITH_MESSAGE { ERROR(STRING("Could not find an RSDP!\n")); }
    }

    KFLUSH_AFTER {
        INFO(STRING("Prepared and collected all necessary information to jump "
                    "to the kernel.\n"));
    }

    MemoryInfo memoryInfo = getMemoryInfo();
    KernelMemory stub = stubMemoryBeforeExitBootServices(&memoryInfo);

    // NOTE: Keep this call in between the stub and the creation of available
    // memory! The stub allocates memory and logs on failure which is not
    // permissible after we have exited boot services
    status = globals.st->boot_services->exit_boot_services(globals.h,
                                                           memoryInfo.mapKey);
    EXIT_WITH_MESSAGE_IF(status) {
        ERROR(STRING("could not exit boot services!\n"));
        ERROR(STRING("Am I running on a buggy implementation that needs to "
                     "call exit boot services twice?\n"));
    }

    params->memory = convertToKernelMemory(&memoryInfo, stub);

    jumpIntoKernel(STACK_START);
    return !SUCCESS;
}
