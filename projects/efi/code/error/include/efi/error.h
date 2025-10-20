#ifndef EFI_ERROR_H
#define EFI_ERROR_H

#include "efi/firmware/base.h"
#include "efi/firmware/graphics-output.h"
#include "shared/macros.h"

void waitKeyThenReset();

typedef enum {
    START = 0,
    PHYSICAL_MEMORY_INITED,
    BOOT_SERVICES_EXITED,
    PHYSICAL_MEMORY_COLLECTED,
} StageNoConsoleOut;

void drawStatusRectangle(GraphicsOutputProtocolMode *mode, U32 color);
void stageStatusUpdate(GraphicsOutputProtocolMode *mode,
                       StageNoConsoleOut stage);

static constexpr auto RED_COLOR = 0xFF0000;

#define EXIT_WITH_MESSAGE                                                      \
    for (auto MACRO_VAR(i) = 0; MACRO_VAR(i) < 1;                              \
         MACRO_VAR(i) = 1, standardBufferFlush(), waitKeyThenReset())

#define EXIT_WITH_MESSAGE_IF_EFI_ERROR(status)                                 \
    if (EFI_ERROR(status))                                                     \
    EXIT_WITH_MESSAGE

#endif
