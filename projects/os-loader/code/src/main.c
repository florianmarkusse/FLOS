#include "abstraction/efi.h"
#include "abstraction/jmp.h"
#include "abstraction/log.h"
#include "abstraction/memory/virtual/converter.h"
#include "abstraction/memory/virtual/map.h"
#include "abstraction/thread.h"
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
#include "efi/memory/virtual.h"
#include "os-loader/data-reading.h" // for getKernelInfo
#include "os-loader/memory.h"       // for mapMemoryAt
#include "shared/log.h"
#include "shared/maths.h" // for CEILING_DIV_V...
#include "shared/memory/allocator/status/buddy.h"
#include "shared/memory/converter.h"
#include "shared/memory/management/definitions.h"
#include "shared/memory/management/management.h"
#include "shared/memory/management/page.h"
#include "shared/memory/management/status.h"
#include "shared/memory/policy.h"
#include "shared/memory/policy/status.h"
#include "shared/text/string.h"   // for CEILING_DIV_V...
#include "shared/types/numeric.h" // for U64, U32, USize

static constexpr auto KERNEL_PERMANENT_MEMORY = 4 * MiB;
static constexpr auto KERNEL_PERMANENT_MEMORY_ALIGNMENT =
    2 * MiB; // Good enough for optimal mappings.
static constexpr auto KERNEL_TEMPORARY_MEMORY = 4 * MiB;
static constexpr auto MIN_VIRTUAL_MEMORY_REQUIRED = 32 * GiB;

Status efi_main(Handle handle, SystemTable *systemtable) {
    globals.h = handle;
    globals.st = systemtable;
    globals.st->con_out->reset(globals.st->con_out, false);
    globals.st->con_out->set_attribute(globals.st->con_out,
                                       BACKGROUND_BLACK | WHITE);

    void *memoryForArena = allocatePages(DYNAMIC_MEMORY_CAPACITY);

    globals.uefiMemory =
        (Arena){.curFree = memoryForArena,
                .beg = memoryForArena,
                .end = memoryForArena + DYNAMIC_MEMORY_CAPACITY};
    if (setjmp(globals.uefiMemory.jmpBuf)) {
        EXIT_WITH_MESSAGE {
            ERROR(STRING("Ran out of dynamic memory capacity\n"));
        }
    }

    U8 *memoryKernelPermanent = findAlignedMemoryBlock(
        KERNEL_PERMANENT_MEMORY, KERNEL_PERMANENT_MEMORY_ALIGNMENT,
        globals.uefiMemory, false);

    globals.kernelPermanent =
        (Arena){.curFree = memoryKernelPermanent,
                .beg = memoryKernelPermanent,
                .end = memoryKernelPermanent + KERNEL_PERMANENT_MEMORY};
    if (setjmp(globals.kernelPermanent.jmpBuf)) {
        EXIT_WITH_MESSAGE {
            ERROR(STRING("No more permanent kernel memory available\n"));
        }
    }

    U8 *memoryKernelTemporary = findAlignedMemoryBlock(
        KERNEL_TEMPORARY_MEMORY, UEFI_PAGE_SIZE, globals.uefiMemory, false);
    globals.kernelTemporary =
        (Arena){.curFree = memoryKernelTemporary,
                .beg = memoryKernelTemporary,
                .end = memoryKernelTemporary + KERNEL_TEMPORARY_MEMORY};
    if (setjmp(globals.kernelTemporary.jmpBuf)) {
        EXIT_WITH_MESSAGE {
            ERROR(STRING("No more Temporary kernel memory available\n"));
        }
    }

    KFLUSH_AFTER {
        INFO(STRING("Scratch memory:          "));
        memoryAppend((Memory){.start = (U64)memoryForArena,
                              .bytes = DYNAMIC_MEMORY_CAPACITY});
        INFO(STRING("\n"));

        INFO(STRING("Kernel permanent memory: "));
        memoryAppend((Memory){.start = (U64)memoryKernelPermanent,
                              .bytes = KERNEL_PERMANENT_MEMORY});
        INFO(STRING("\n"));

        INFO(STRING("Kernel temporary memory: "));
        memoryAppend((Memory){.start = (U64)memoryKernelTemporary,
                              .bytes = KERNEL_TEMPORARY_MEMORY});
        INFO(STRING("\n"));
    }

    initRootVirtualMemoryInKernel();

    KFLUSH_AFTER { INFO(STRING("Loading kernel...\n")); }
    U32 kernelBytes = getKernelBytes(globals.uefiMemory);
    if (kernelBytes > kernelCodeSizeMax()) {
        EXIT_WITH_MESSAGE {
            ERROR(STRING(
                "\nthe kernel is too large!\nMaximum allowed kernel size: "));
            ERROR(kernelCodeSizeMax(), .flags = NEWLINE);
            ERROR(STRING("Current kernel size: "));
            ERROR(kernelBytes, .flags = NEWLINE);
        }
    }

    String kernelContent =
        readKernelFromCurrentLoadedImage(kernelBytes, globals.uefiMemory);

    if (mapMemory(kernelCodeStart(), (U64)kernelContent.buf, kernelContent.len,
                  pageFlagsReadWrite() | pageFlagsNoCacheEvict()) <
        kernelCodeStart()) {
        EXIT_WITH_MESSAGE {
            ERROR(STRING(
                "Kernel mapping overflowed out of the address space!\n"));
        }
    }

    KFLUSH_AFTER {
        mappingMemoryAppend(kernelCodeStart(), (U64)kernelContent.buf,
                            kernelContent.len);
    }

    KFLUSH_AFTER {
        INFO(STRING("Locating highest physical memory address...\n"));
    }

    GraphicsOutputProtocol *gop = nullptr;
    Status status = globals.st->boot_services->locate_protocol(
        &GRAPHICS_OUTPUT_PROTOCOL_GUID, nullptr, (void **)&gop);
    EXIT_WITH_MESSAGE_IF_EFI_ERROR(status) {
        ERROR(STRING("Could not locate locate GOP\n"));
    }

    U64 highestLowerHalfAddress = findHighestMemoryAddress(
        gop->mode->frameBufferBase + gop->mode->frameBufferSize,
        globals.uefiMemory);
    KFLUSH_AFTER {
        INFO(STRING("Identity mapping all memory, highest address found: "));
        INFO((void *)highestLowerHalfAddress, .flags = NEWLINE);
    }
    U64 firstFreeVirtual =
        mapMemory(0, 0, highestLowerHalfAddress,
                  pageFlagsReadWrite() | pageFlagsNoCacheEvict());

    initKernelMemoryManagement(firstFreeVirtual, kernelVirtualMemoryEnd());

    U64 virtualForKernel = (U64)allocVirtualMemory(MIN_VIRTUAL_MEMORY_REQUIRED);
    U64 endVirtualForKernel = virtualForKernel + MIN_VIRTUAL_MEMORY_REQUIRED;

    KFLUSH_AFTER {
        INFO(STRING("Virtual memory for UEFI: "));
        memoryAppend((Memory){.start = (U64)virtualForKernel,
                              .bytes = MIN_VIRTUAL_MEMORY_REQUIRED});
        INFO(STRING("\n"));
    }

    // NOTE: Virtual memory active from this point!

    KFLUSH_AFTER { INFO(STRING("Mapping output window...\n")); }
    virtualForKernel =
        alignVirtual(virtualForKernel, gop->mode->frameBufferBase,
                     gop->mode->frameBufferSize);
    U64 screenMemoryVirtualStart = virtualForKernel;
    virtualForKernel =
        mapMemory(virtualForKernel, gop->mode->frameBufferBase,
                  gop->mode->frameBufferSize,
                  pageFlagsReadWrite() | pageFlagsNoCacheEvict() |
                      pageFlagsScreenMemory());

    KFLUSH_AFTER {
        mappingMemoryAppend(screenMemoryVirtualStart,
                            gop->mode->frameBufferBase,
                            gop->mode->frameBufferSize);
    }

    KFLUSH_AFTER { INFO(STRING("Setting up thread stack...\n")); }
    StackResult stackResult =
        stackCreateAndMap(virtualForKernel, KERNEL_STACK_SIZE, true);
    virtualForKernel = stackResult.virtualMemoryFirstAvailable;

    KFLUSH_AFTER { INFO(STRING("Setting up kernel parameters...\n")); }

    U64 kernelParamsSize = sizeof(KernelParameters) + ARCH_PARAMS_SIZE;
    KernelParameters *kernelParams = (KernelParameters *)NEW(
        &globals.kernelTemporary, U8, .count = kernelParamsSize,
        .align = alignof(KernelParameters));

    KFLUSH_AFTER {
        memoryAppend(
            (Memory){.start = (U64)kernelParams, .bytes = kernelParamsSize});
        INFO(STRING("\n"));
    }

    kernelParams->window =
        (Window){.pixels = (U32 *)screenMemoryVirtualStart,
                 .size = gop->mode->frameBufferSize,
                 .width = gop->mode->info->horizontalResolution,
                 .height = gop->mode->info->verticalResolution,
                 .scanline = gop->mode->info->pixelsPerScanLine};

    KFLUSH_AFTER { INFO(STRING("Filling specific arch params...\n")); }
    fillArchParams(kernelParams->archParams, virtualForKernel);

    RSDPResult rsdp = getRSDP(globals.st->number_of_table_entries,
                              globals.st->configuration_table);
    if (!rsdp.rsdp) {
        EXIT_WITH_MESSAGE { ERROR(STRING("Could not find an RSDP!\n")); }
    }

    if (virtualForKernel >= endVirtualForKernel) {
        EXIT_WITH_MESSAGE {
            ERROR(STRING("Kernel virtual memory exceeded maximum allowed "
                         "memory\nHow did this happen? Allow more virtual "
                         "memory to OS-loader or bug?"));
        }
    }

    // NOTE: Don't use virtual memory allocations anymore from this point
    // onward.
    kernelParams->memory.buddyVirtual = buddyVirtual.data;
    kernelParams->memory.memoryMapperSizes = memoryMapperSizes;

    KFLUSH_AFTER {
        INFO(STRING("Finished set-up. Collecting physical memory and jumping "
                    "to the kernel. Setting up a square in the top-left corner "
                    "that indicates the status.\nGreen: Good\nRed: Bad\n"));
    }
    stageStatusUpdate(gop->mode, START);

    Buddy uefiBuddyPhysical;
    kernelPhysicalBuddyPrepare(&uefiBuddyPhysical, firstFreeVirtual, gop->mode);

    stageStatusUpdate(gop->mode, PHYSICAL_MEMORY_INITED);

    kernelParams->permanentLeftoverFree =
        (Memory){.start = (U64)globals.kernelPermanent.curFree,
                 .bytes = (U64)(globals.kernelPermanent.end -
                                globals.kernelPermanent.curFree)};
    kernelParams->self = (Memory){.start = (U64)globals.kernelTemporary.beg,
                                  .bytes = KERNEL_TEMPORARY_MEMORY};

    /* NOTE: Keep this call in between the stub and the creation of available */
    /* memory! The stub allocates memory and logs on failure which is not */
    /* permissible after we have exited boot services */
    MemoryInfo memoryInfo = getMemoryInfo(&globals.uefiMemory);
    status = globals.st->boot_services->exit_boot_services(globals.h,
                                                           memoryInfo.mapKey);
    EXIT_WITH_MESSAGE_IF_EFI_ERROR(status) {
        ERROR(STRING("could not exit boot services!\nAm I running on a buggy "
                     "implementation that needs to "
                     "call exit boot services twice?\n"));
    }

    stageStatusUpdate(gop->mode, BOOT_SERVICES_EXITED);

    convertToKernelMemory(&memoryInfo, &uefiBuddyPhysical,
                          &kernelParams->memory.physicalMemoryTotal);
    kernelParams->memory.buddyPhysical = uefiBuddyPhysical.data;

    stageStatusUpdate(gop->mode, PHYSICAL_MEMORY_COLLECTED);

    jumpIntoKernel(stackResult.stackVirtualTop, 0, kernelParams);

    __builtin_unreachable();
}
