#include "abstraction/kernel.h"

#include "abstraction/interrupts.h"
#include "x86/configuration/cpu.h"
#include "x86/efi-to-kernel/params.h"
#include "x86/idt.h"
#include "x86/memory/definitions.h"
#include "x86/memory/virtual.h"
#include "x86/serial.h"
#include "x86/time.h"

void archInit(void *archParams) {
    X86ArchParams *x86ArchParams = (X86ArchParams *)archParams;
    XSAVESpace = x86ArchParams->XSAVELocation;
    initIDT();
    cyclesPerMicroSecond = x86ArchParams->tscFrequencyPerMicroSecond;

    rootPageTable = (VirtualPageTable *)CR3();

    rootPageMetaData.children =
        (PageMetaDataNode *)x86ArchParams->rootPageMetaData.children;
    rootPageMetaData.metaData.entriesMapped =
        x86ArchParams->rootPageMetaData.metaData.entriesMapped;
    rootPageMetaData.metaData.entriesMappedWithSmallerGranularity =
        x86ArchParams->rootPageMetaData.metaData
            .entriesMappedWithSmallerGranularity;

    // TODO: This is just for serial output, should we ifdef this or something?
    outb(COM1 + 1, 0x00); // Disable interrupts
    outb(COM1 + 3, 0x80); // Enable DLAB
    outb(COM1 + 0, 0x03); // Baud rate low byte (38400 baud)
    outb(COM1 + 1, 0x00); // Baud rate high byte
    outb(COM1 + 3, 0x03); // 8 bits, no parity, one stop bit
    outb(COM1 + 2, 0xC7); // Enable FIFO, clear them, 14-byte threshold
    outb(COM1 + 4, 0x0B); // IRQs enabled, RTS/DSR set
}
