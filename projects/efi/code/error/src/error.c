#include "efi/error.h"
#include "efi/firmware/graphics-output.h"
#include "efi/firmware/simple-text-input.h"
#include "efi/firmware/system.h"
#include "efi/globals.h"

void waitKeyThenReset() {
    InputKey key;
    while (globals.st->con_in->read_key_stroke(globals.st->con_in, &key) !=
           SUCCESS) {
        ;
    }
    globals.st->runtime_services->reset_system(RESET_SHUTDOWN, SUCCESS, 0,
                                               nullptr);
}

static U32 callCount = 0;

static U32 STAGE_COLOR_START = 0x003300;
static U32 STAGE_COLOR_INCREMENT = 0x003300;

static constexpr auto pixelSideCount = 40;
void drawStatusRectangle(GraphicsOutputProtocolMode *mode, U32 color) {
    U32 *screen = ((U32 *)mode->frameBufferBase);
    U32 start = callCount * pixelSideCount;
    for (U32 x = start; x < start + pixelSideCount; x++) {
        for (U32 y = 0; y < pixelSideCount; y++) {
            screen[y * mode->info->pixelsPerScanLine + x] = color;
        }
    }
    callCount++;
}

void stageStatusUpdate(GraphicsOutputProtocolMode *mode,
                       StageNoConsoleOut stage) {
    drawStatusRectangle(mode, STAGE_COLOR_START +
                                  ((U32)stage * STAGE_COLOR_INCREMENT));
}
