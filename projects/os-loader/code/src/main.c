#include "abstraction/efi.h"
#include "abstraction/jmp.h"
#include "abstraction/log.h"
#include "abstraction/memory/physical.h"
#include "abstraction/memory/virtual/converter.h"
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
#include "efi/memory/physical.h"
#include "os-loader/data-reading.h" // for getKernelInfo
#include "os-loader/memory.h"       // for mapMemoryAt
#include "shared/log.h"
#include "shared/maths/maths.h" // for CEILING_DIV_V...
#include "shared/memory/converter.h"
#include "shared/memory/management/definitions.h"
#include "shared/memory/virtual.h"
#include "shared/text/string.h" // for CEILING_DIV_V...
#include "shared/types/numeric.h" // for U64, U32, USize

static U64 kernelFreeVirtualMemory = HIGHER_HALF_START;

Status efi_main(Handle handle, SystemTable *systemtable) {
    globals.h = handle;
    globals.st = systemtable;
    globals.st->con_out->reset(globals.st->con_out, false);
    globals.st->con_out->set_attribute(globals.st->con_out,
                                       BACKGROUND_RED | YELLOW);

    void *memoryForArena =
        (void *)allocateUnalignedMemory(DYNAMIC_MEMORY_CAPACITY, false);
    Arena arena = (Arena){.curFree = memoryForArena,
                          .beg = memoryForArena,
                          .end = memoryForArena + DYNAMIC_MEMORY_CAPACITY};
    if (setjmp(arena.jmp_buf)) {
        EXIT_WITH_MESSAGE {
            ERROR(STRING("Ran out of dynamic memory capacity\n"));
        }
    }
    initKernelStructureLocations(&arena);

    initArchitecture(arena);

    KFLUSH_AFTER { INFO(STRING("Going to fetch kernel bytes\n")); }
    U64 kernelBytes = getKernelBytes(arena);
    if (kernelBytes > KERNEL_CODE_MAX_SIZE) {
        EXIT_WITH_MESSAGE {
            ERROR(STRING(
                "the kernel is too large!\nMaximum allowed kernel size: "));
            ERROR(KERNEL_CODE_MAX_SIZE, NEWLINE);
            ERROR(STRING("Current kernel size: "));
            ERROR(kernelBytes, NEWLINE);
        }
    }
    KFLUSH_AFTER {
        INFO(STRING("Loading kernel into memory\n"));
        INFO(STRING("Bytes: "));
        INFO(kernelBytes, NEWLINE);
    }

    string kernelContent = readKernelFromCurrentLoadedImage(kernelBytes, arena);

    KFLUSH_AFTER { INFO(STRING("Mapping kernel into location\n")); }
    if (mapMemory(KERNEL_CODE_START, (U64)kernelContent.buf,
                  kernelContent.len) < KERNEL_CODE_START) {
        EXIT_WITH_MESSAGE {
            ERROR(STRING(
                "Kernel mapping overflowed out of the address space!\n"));
        }
    }

    KFLUSH_AFTER {
        INFO(STRING("The phyiscal kernel:\nstart: "));
        INFO((void *)kernelContent.buf, NEWLINE);
        INFO(STRING("stop:  "));
        INFO((void *)(kernelContent.buf + kernelContent.len), NEWLINE);

        INFO(STRING("The virtual kernel:\nstart: "));
        INFO((void *)KERNEL_CODE_START, NEWLINE);
        INFO(STRING("stop:  "));
        INFO((void *)(KERNEL_CODE_START + kernelContent.len), NEWLINE);
    }

    GraphicsOutputProtocol *gop = nullptr;
    Status status = globals.st->boot_services->locate_protocol(
        &GRAPHICS_OUTPUT_PROTOCOL_GUID, nullptr, (void **)&gop);
    EXIT_WITH_MESSAGE_IF_EFI_ERROR(status) {
        ERROR(STRING("Could not locate locate GOP\n"));
    }

    U64 highestLowerHalfAddress = findHighestMemoryAddress(
        gop->mode->frameBufferBase + gop->mode->frameBufferSize, arena);
    KFLUSH_AFTER {
        INFO(STRING("Identity mapping all memory, highest address found: "));
        INFO((void *)highestLowerHalfAddress, NEWLINE);
    }
    mapMemory(0, 0, highestLowerHalfAddress);

    KFLUSH_AFTER { INFO(STRING("Mapping screen memory into location\n")); }
    kernelFreeVirtualMemory =
        alignVirtual(kernelFreeVirtualMemory, gop->mode->frameBufferBase,
                     gop->mode->frameBufferSize);
    U64 screenMemoryVirtualStart = kernelFreeVirtualMemory;
    kernelFreeVirtualMemory = mapMemoryWithFlags(
        kernelFreeVirtualMemory, gop->mode->frameBufferBase,
        gop->mode->frameBufferSize,
        KERNEL_STANDARD_PAGE_FLAGS | KERNEL_SCREEN_MEMORY_PAGE_FLAGS);

    KFLUSH_AFTER {
        INFO(STRING("The graphics buffer physical location:\nstart: "));
        INFO((void *)gop->mode->frameBufferBase, NEWLINE);
        INFO(STRING("stop:  "));
        INFO((void *)(gop->mode->frameBufferBase + gop->mode->frameBufferSize),
             NEWLINE);

        INFO(STRING("The graphics buffer virtual location:\nstart: "));
        INFO((void *)screenMemoryVirtualStart, NEWLINE);
        INFO(STRING("stop:  "));
        INFO((void *)(screenMemoryVirtualStart + gop->mode->frameBufferSize),
             NEWLINE);
        INFO(STRING("virt free memory is now at:     "));
        INFO((void *)kernelFreeVirtualMemory, NEWLINE);
    }

    KFLUSH_AFTER { INFO(STRING("Allocating space for stack\n")); }
    U64 stackAddress =
        allocateKernelStructure(KERNEL_STACK_SIZE, 0, true, arena);

    KFLUSH_AFTER { INFO(STRING("Mapping stack into location\n")); }
    // NOTE: Overflow precaution
    kernelFreeVirtualMemory += KERNEL_STACK_SIZE;
    kernelFreeVirtualMemory =
        alignVirtual(kernelFreeVirtualMemory, stackAddress, KERNEL_STACK_SIZE);

    U64 stackVirtualStart = kernelFreeVirtualMemory;
    kernelFreeVirtualMemory =
        mapMemory(kernelFreeVirtualMemory, stackAddress, KERNEL_STACK_SIZE);

    KFLUSH_AFTER {
        INFO(STRING("The phyiscal stack:\ndown from: "));
        INFO((void *)stackAddress + KERNEL_STACK_SIZE, NEWLINE);
        INFO(STRING("until:     "));
        INFO((void *)stackAddress, NEWLINE);

        INFO(STRING("The virtual stack:\ndown from: "));
        INFO((void *)stackVirtualStart + KERNEL_STACK_SIZE, NEWLINE);
        INFO(STRING("until:     "));
        INFO((void *)stackVirtualStart, NEWLINE);
        INFO(STRING("virt free memory is now at:     "));
        INFO((void *)kernelFreeVirtualMemory, NEWLINE);
    }

    KFLUSH_AFTER { INFO(STRING("Allocating space for kernel parameters\n")); }
    KernelParameters *kernelParams =
        (KernelParameters *)allocateKernelStructure(
            sizeof(KernelParameters), alignof(KernelParameters), false, arena);
    void *kernelParamsEnd = ((U8 *)kernelParams) + sizeof(KernelParameters);

    KFLUSH_AFTER {
        INFO(STRING("The phyiscal kernel params:\nstart: "));
        INFO((void *)kernelParams, NEWLINE);
        INFO(STRING("stop:  "));
        INFO(kernelParamsEnd, NEWLINE);

        INFO(STRING("The virtual kernel params (identity mapped):\nstart: "));
        INFO((void *)kernelParams, NEWLINE);
        INFO(STRING("stop:  "));
        INFO(kernelParamsEnd, NEWLINE);
    }

    kernelParams->memory.virt.freeVirtualMemory = freeVirtualMemory;

    kernelParams->window =
        (Window){.screen = (U32 *)screenMemoryVirtualStart,
                 .size = gop->mode->frameBufferSize,
                 .width = gop->mode->info->horizontalResolution,
                 .height = gop->mode->info->verticalResolution,
                 .scanline = gop->mode->info->pixelsPerScanLine};

    RSDPResult rsdp = getRSDP(globals.st->number_of_table_entries,
                              globals.st->configuration_table);
    if (!rsdp.rsdp) {
        EXIT_WITH_MESSAGE { ERROR(STRING("Could not find an RSDP!\n")); }
    }

    KFLUSH_AFTER {
        INFO(STRING("Finished set-up. Collecting physical memory and jumping "
                    "to the kernel.\n"));
    }

    allocateSpaceForKernelMemory(arena, &kernelParams->memory.physical);

    /* NOTE: Keep this call in between the stub and the creation of available */
    /* memory! The stub allocates memory and logs on failure which is not */
    /* permissible after we have exited boot services */
    MemoryInfo memoryInfo = getMemoryInfo(&arena);
    status = globals.st->boot_services->exit_boot_services(globals.h,
                                                           memoryInfo.mapKey);
    EXIT_WITH_MESSAGE_IF_EFI_ERROR(status) {
        ERROR(STRING("could not exit boot services!\n"));
        ERROR(STRING("Am I running on a buggy implementation that needs to "
                     "call exit boot services twice?\n"));
    }

    convertToKernelMemory(&memoryInfo, &kernelParams->memory.physical);

    jumpIntoKernel(stackVirtualStart + KERNEL_STACK_SIZE, kernelParams);

    __builtin_unreachable();
}
