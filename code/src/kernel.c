/*
 * mykernel/c/kernel.c
 *
 * Copyright (C) 2017 - 2021 bzt (bztsrc@gitlab)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * This file is part of the BOOTBOOT Protocol package.
 * @brief A sample BOOTBOOT compatible kernel
 *
 */

/* function to display a string, see below */
void puts(unsigned char *s);

#include "bootboot.h"
#include "types.h"
#include "util/log.h"

/* imported virtual addresses, see linker script */
extern BOOTBOOT bootboot; // see bootboot.h
extern unsigned char
    environment[4096]; // configuration, UTF-8 text key=value pairs
extern uint8_t fb;     // linear framebuffer mapped

#define HAXOR_GREEN 0x0000FF00
#define BYTES_PER_PIXEL 4

/******************************************
 * Entry point, called by BOOTBOOT Loader *
 ******************************************/
void _start() {
    /*** NOTE: this code runs on all cores in parallel ***/
    int x, y, s = bootboot.fb_scanline, w = bootboot.fb_width,
              h = bootboot.fb_height;

    if (s) {
        for (y = 0; y < w; y++) {
            *((uint32_t *)(&fb + y * BYTES_PER_PIXEL)) = HAXOR_GREEN;
        }
        for (y = 0; y < w; y++) {
            *((uint32_t *)(&fb + (h - 1) * s + y * BYTES_PER_PIXEL)) =
                HAXOR_GREEN;
        }
        for (x = 0; x < h; x++) {
            *((uint32_t *)(&fb + s * x)) = HAXOR_GREEN;
        }
        for (x = 0; x < h; x++) {
            *((uint32_t *)(&fb + s * x + (w - 1) * BYTES_PER_PIXEL)) =
                HAXOR_GREEN;
        }

        // cross-hair to see screen dimension detected correctly
        //    for (y = 0; y < h; y++) {
        //      *((uint32_t *)(&fb + s * y + (w * 2))) = 0x00FFFFFF;
        //    }
        //    for (x = 0; x < w; x++) {
        //      *((uint32_t *)(&fb + s * (h / 2) + x * 4)) = 0x00FFFFFF;
        //    }
        //
        //    // red, green, blue boxes in order
        //    for (y = 0; y < 20; y++) {
        //      for (x = 0; x < 20; x++) {
        //        *((uint32_t *)(&fb + s * (y + 20) + (x + 20) * 4)) =
        //        0x00FF0000;
        //      }
        //    }
        //    for (y = 0; y < 20; y++) {
        //      for (x = 0; x < 20; x++) {
        //        *((uint32_t *)(&fb + s * (y + 20) + (x + 50) * 4)) =
        //        HAXOR_GREEN;
        //      }
        //    }
        //    for (y = 0; y < 20; y++) {
        //      for (x = 0; x < 20; x++) {
        //        *((uint32_t *)(&fb + s * (y + 20) + (x + 80) * 4)) =
        //        0x000000FF;
        //      }
        //    }

        // say hello
        puts((unsigned char *)"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
        puts((unsigned char *)"gggggggggggggggggggggggggggggggggggggggg");
        puts((unsigned char *)"dddddddddddddddddddddddddddddddddddddddd");
        puts((
            unsigned char
                *)"HelloooooooooooTTTTTLLLLLLLoo from a simple "
                  "BOOTBOOTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT"
                  "TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTtT hdfkjgh dfkh "
                  "gkdhfkj ghdfkjl hgkdfhlkdfh glkjfdh gld hldfkh gldkfj hldf");
    }

    printf("Attempting to print free memory...\n");
    MMapEnt *mmap_ent = &bootboot.mmap;
    while ((uint64_t)mmap_ent < (uint64_t)((char *)&bootboot + bootboot.size)) {
        printf("location: %d size: %d\n", mmap_ent->ptr, mmap_ent->size);
        mmap_ent++;
    }

    // hang for now
    while (1)
        ;
}

// The header contains all the data for each glyph. After that comes numGlyph *
// bytesPerGlyph bytes.
//            padding
//  Font data    |
// +----------+ +--+
// 000001100000 0000
// 000011110000 0000
// 000110011000 0000
// 001100001100 0000
// 011000000110 0000
// 110000000011 0000
// 111111111111 0000
// 111111111111 0000
// 110000000011 0000
// 110000000011 0000
// 110000000011 0000
// 110000000011 0000
typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t headersize;
    uint32_t flags;
    uint32_t numglyph;
    uint32_t bytesperglyph;
    uint32_t height;
    uint32_t width;
    uint8_t glyphs;
} __attribute__((packed)) psf2_t;
extern unsigned char glyphStart[] asm("_binary_resources_font_psf_start");

#define HORIZONTAL_PADDING 0
#define PIXEL_MARGIN 20
void puts(unsigned char *s) {
    static psf2_t *font = (psf2_t *)&glyphStart;
    static uint64_t typedGlyphs = 0;
    uint64_t glyphsPerLine = (bootboot.fb_width - PIXEL_MARGIN * 2) /
                             (font->width + HORIZONTAL_PADDING);
    uint32_t glyphsOnLine = 0;
    int bytesPerLine = (font->width + 7) / 8;
    while (*s) {
        unsigned char *glyph =
            (unsigned char *)&glyphStart + font->headersize +
            (*s > 0 && *s < font->numglyph ? *s : 0) * font->bytesperglyph;
        uint32_t offset = (glyphsOnLine / glyphsPerLine) *
                              (bootboot.fb_scanline * font->height) +
                          (glyphsOnLine % glyphsPerLine) *
                              (font->width + HORIZONTAL_PADDING) *
                              BYTES_PER_PIXEL;
        for (uint32_t y = 0; y < font->height; y++) {
            // TODO: use SIMD instructions?
            uint32_t line = offset;
            uint32_t mask = 1 << (font->width - 1);
            for (uint32_t x = 0; x < font->width; x++) {
                // NOLINTNEXTLINE
                *((uint32_t *)((uint64_t)&fb +
                               (PIXEL_MARGIN * bootboot.fb_scanline) +
                               (PIXEL_MARGIN * BYTES_PER_PIXEL) + line)) =
                    ((((uint32_t)*glyph) & (mask)) != 0) * 0xFFFFFF;

                mask >>= 1;
                line += BYTES_PER_PIXEL;
            }
            glyph += bytesPerLine;
            offset += bootboot.fb_scanline;
        }
        s++;
        glyphsOnLine++;
    }
}
