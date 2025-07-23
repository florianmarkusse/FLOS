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

void drawStatusRectangle(GraphicsOutputProtocolMode *mode, U32 color) {
    U32 *screen = ((U32 *)mode->frameBufferBase);
    for (U64 x = 0; x < 10; x++) {
        for (U64 y = 0; y < 10; y++) {
            screen[y * mode->info->pixelsPerScanLine + x] = color;
        }
    }
}
