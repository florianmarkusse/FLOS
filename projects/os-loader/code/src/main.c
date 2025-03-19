#include "abstraction/efi.h"
#include "abstraction/jmp.h"
#include "abstraction/log.h"
#include "abstraction/memory/physical/allocation.h"
#include "abstraction/memory/virtual/map.h"
#include "efi-to-kernel/kernel-parameters.h"  // for KernelParameters
#include "efi-to-kernel/memory/definitions.h" // for STACK_SIZE
#include "efi/acpi/rdsp.h"                    // for getRSDP, RSDP...
#include "efi/error.h"
#include "efi/firmware/base.h" // for PhysicalAddress
#include "efi/firmware/block-io.h"
#include "efi/firmware/graphics-output.h" // for GRAPHICS_OUTP...
#include "efi/firmware/simple-text-input.h"
#include "efi/firmware/simple-text-output.h" // for SimpleTextOut...
#include "efi/firmware/system.h"             // for PhysicalAddress
#include "efi/globals.h"                     // for globals
#include "efi/memory/definitions.h"
#include "efi/memory/physical.h"
#include "efi/memory/virtual.h"
#include "os-loader/data-reading.h" // for getKernelInfo
#include "os-loader/memory.h"       // for mapMemoryAt
#include "shared/log.h"
#include "shared/maths/maths.h" // for CEILING_DIV_V...
#include "shared/memory/converter.h"
#include "shared/memory/management/definitions.h"
#include "shared/text/string.h" // for CEILING_DIV_V...
#include "shared/types/types.h" // for U64, U32, USize

Status efi_main(Handle handle, SystemTable *systemtable) {
    globals.h = handle;
    globals.st = systemtable;
    globals.st->con_out->reset(globals.st->con_out, false);
    globals.st->con_out->set_attribute(globals.st->con_out,
                                       BACKGROUND_RED | YELLOW);

    void *memoryForArena = (void *)getAlignedPhysicalMemory(
        DYNAMIC_MEMORY_CAPACITY, DYNAMIC_MEMORY_ALIGNMENT);
    Arena arena = (Arena){.curFree = memoryForArena,
                          .beg = memoryForArena,
                          .end = memoryForArena + DYNAMIC_MEMORY_CAPACITY};
    if (setjmp(arena.jmp_buf)) {
        EXIT_WITH_MESSAGE {
            ERROR(STRING("Ran out of dynamic memory capacity\n"));
        }
    }
    initVirtualMemoryMapper(getAlignedPhysicalMemoryWithArena(
        VIRTUAL_MEMORY_MAPPER_CAPACITY, VIRTUAL_MEMORY_MAPPER_ALIGNMENT,
        arena));

    initArchitecture(arena);

    KFLUSH_AFTER { INFO(STRING("gOING TO READ KERNEL INFO\n")); }
    KFLUSH_AFTER { INFO(STRING("Going to read kernel info\n")); }
    DataPartitionFile kernelFile = getKernelInfo(arena);
    if (kernelFile.bytes > KERNEL_CODE_MAX_ALIGNMENT) {
        EXIT_WITH_MESSAGE {
            ERROR(STRING("the size of the kernel code is too large!\n"));
            ERROR(STRING("Maximum allowed size: "));
            ERROR(KERNEL_CODE_MAX_ALIGNMENT, NEWLINE);
            ERROR(STRING("Current size: "));
            ERROR(kernelFile.bytes, NEWLINE);
        }
    }
    KFLUSH_AFTER {
        INFO(STRING("Loading kernel into memory\n"));
        INFO(STRING("Bytes: "));
        INFO(kernelFile.bytes, NEWLINE);
        INFO(STRING("LBA: "));
        INFO(kernelFile.lbaStart, NEWLINE);
    }
    string kernelContent = readDiskLbasFromCurrentLoadedImage(
        kernelFile.lbaStart, kernelFile.bytes, arena);
    KFLUSH_AFTER {
        INFO(STRING("The phyiscal kernel location:\n"));
        INFO((void *)kernelContent.buf, NEWLINE);
        INFO((void *)(kernelContent.buf + kernelContent.len), NEWLINE);
    }

    EXIT_WITH_MESSAGE {
        ERROR(
            STRING("------------------------------------------------------\n"));
    }

    /**/
    /*KFLUSH_AFTER { INFO(STRING("Mapping kernel into location\n")); }*/
    /*mapWithSmallestNumberOfPagesInKernelMemory(*/
    /*    KERNEL_CODE_START, (U64)kernelContent.buf, kernelFile.bytes);*/
    /*KFLUSH_AFTER {*/
    /*    INFO(STRING("The phyiscal kernel goes from: "));*/
    /*    INFO((void *)kernelContent.buf, NEWLINE);*/
    /*    INFO(STRING("to: "));*/
    /*    INFO((void *)(kernelContent.buf + kernelContent.len), NEWLINE);*/
    /**/
    /*    INFO(STRING("The virtual kernel goes from: "));*/
    /*    INFO((void *)KERNEL_CODE_START, NEWLINE);*/
    /*    INFO(STRING("to: "));*/
    /*    INFO((void *)(KERNEL_CODE_START + kernelContent.len), NEWLINE);*/
    /*}*/
    /**/
    /*GraphicsOutputProtocol *gop = nullptr;*/
    /*Status status = globals.st->boot_services->locate_protocol(*/
    /*    &GRAPHICS_OUTPUT_PROTOCOL_GUID, nullptr, (void **)&gop);*/
    /*EXIT_WITH_MESSAGE_IF(status) {*/
    /*    ERROR(STRING("Could not locate locate GOP\n"));*/
    /*}*/
    /**/
    /*KFLUSH_AFTER {*/
    /*    INFO(STRING("The graphics buffer location is at: "));*/
    /*    INFO((void *)gop->mode->frameBufferBase, NEWLINE);*/
    /*    INFO(STRING("The graphics buffer size is: "));*/
    /*    INFO(gop->mode->frameBufferSize, NEWLINE);*/
    /*}*/
    /**/
    /*KFLUSH_AFTER { INFO(STRING("Identity mapping all memory\n")); }*/
    /*identityMapPhysicalMemory(gop->mode->frameBufferBase +*/
    /*                          gop->mode->frameBufferSize);*/
    /**/
    /*KFLUSH_AFTER { INFO(STRING("Allocating space for stack\n")); }*/
    /*PhysicalAddress stackStart =*/
    /*    allocate4KiBPages(CEILING_DIV_VALUE(KERNEL_STACK_SIZE,
     * UEFI_PAGE_SIZE));*/
    /*mapWithSmallestNumberOfPagesInKernelMemory(KERNEL_STACK_START,
     * stackStart,*/
    /*                                           KERNEL_STACK_SIZE);*/
    /*KFLUSH_AFTER {*/
    /*    INFO(STRING("The phyiscal stack will go down from: "));*/
    /*    INFO((void *)stackStart + KERNEL_STACK_SIZE, NEWLINE);*/
    /*    INFO(STRING("to: "));*/
    /*    INFO((void *)stackStart, NEWLINE);*/
    /**/
    /*    INFO(STRING("The virtual stack will go down from: "));*/
    /*    INFO((void *)KERNEL_STACK_START + KERNEL_STACK_SIZE, NEWLINE);*/
    /*    INFO(STRING("to: "));*/
    /*    INFO((void *)KERNEL_STACK_START, NEWLINE);*/
    /*}*/
    /**/
    /*KFLUSH_AFTER { INFO(STRING("Allocating space for kernel parameters\n"));
     * }*/
    /*PhysicalAddress kernelParams = allocate4KiBPages(*/
    /*    CEILING_DIV_VALUE(KERNEL_PARAMS_SIZE, UEFI_PAGE_SIZE));*/
    /*mapWithSmallestNumberOfPagesInKernelMemory(*/
    /*    KERNEL_PARAMS_START, kernelParams, KERNEL_PARAMS_SIZE);*/
    /*KFLUSH_AFTER {*/
    /*    INFO(STRING("The phyiscal kernel params goes from: "));*/
    /*    INFO((void *)kernelParams, NEWLINE);*/
    /*    INFO(STRING("to: "));*/
    /*    INFO((void *)kernelParams + KERNEL_PARAMS_SIZE, NEWLINE);*/
    /**/
    /*    INFO(STRING("The virtual kernel params goes from: "));*/
    /*    INFO((void *)KERNEL_PARAMS_START, NEWLINE);*/
    /*    INFO(STRING("to: "));*/
    /*    INFO((void *)KERNEL_PARAMS_START + KERNEL_PARAMS_SIZE, NEWLINE);*/
    /*}*/

    /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
    /*KernelParameters *params = (KernelParameters *)kernelParams;*/
    /**/
    /*params->fb.columns = gop->mode->info->horizontalResolution;*/
    /*params->fb.rows = gop->mode->info->verticalResolution;*/
    /*params->fb.scanline = gop->mode->info->pixelsPerScanLine;*/
    /*params->fb.ptr = gop->mode->frameBufferBase;*/
    /*params->fb.size = gop->mode->frameBufferSize;*/
    /**/
    /*RSDPResult rsdp = getRSDP(globals.st->number_of_table_entries,*/
    /*                          globals.st->configuration_table);*/
    /*if (!rsdp.rsdp) {*/
    /*    EXIT_WITH_MESSAGE { ERROR(STRING("Could not find an RSDP!\n")); }*/
    /*}*/
    /**/
    /*KFLUSH_AFTER {*/
    /*    INFO(STRING("Prepared and collected all necessary information to jump
     * "*/
    /*                "to the kernel.\n"));*/
    /*}*/
    /**/
    /*MemoryInfo memoryInfo = getMemoryInfo();*/
    /*KernelMemory stub = stubMemoryBeforeExitBootServices(&memoryInfo);*/
    /**/
    /*// NOTE: Keep this call in between the stub and the creation of
     * available*/
    /*// memory! The stub allocates memory and logs on failure which is not*/
    /*// permissible after we have exited boot services*/
    /*status = globals.st->boot_services->exit_boot_services(globals.h,*/
    /*                                                       memoryInfo.mapKey);*/
    /*EXIT_WITH_MESSAGE_IF(status) {*/
    /*    ERROR(STRING("could not exit boot services!\n"));*/
    /*    ERROR(STRING("Am I running on a buggy implementation that needs to "*/
    /*                 "call exit boot services twice?\n"));*/
    /*}*/
    /**/
    /*params->memory = convertToKernelMemory(&memoryInfo, stub);*/
    /**/
    /*jumpIntoKernel(KERNEL_STACK_START + KERNEL_STACK_SIZE);*/
    /**/
    /*__builtin_unreachable();*/
}
