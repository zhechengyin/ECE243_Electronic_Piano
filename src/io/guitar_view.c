#include "io/guitar_view.h"

#include "io/vga_core.h"
#include "io/vga.h"

void guitar_view_draw_static(void) {
    volatile char *buf = (volatile char *)VGA_CHAR_BUFFER;

    vga_core_clear_char_buffer();
    vga_core_clear_screen(COLOR_BG);

    {
        const char *mode  = "MODE: GUITAR";
        const char *title = "GUITAR FRETBOARD";
        const char *open  = "OPEN: E2 A2 D3 G3 B3 E4";
        const char *range = "RANGE: E2 - E6";

        int off;

        off = (0 << 7) + 2;
        while (*mode)  buf[off++] = *mode++;

        off = (1 << 7) + ((CHAR_W - 16) / 2);
        while (*title) buf[off++] = *title++;

        off = (2 << 7) + 2;
        while (*open)  buf[off++] = *open++;

        off = (3 << 7) + 2;
        while (*range) buf[off++] = *range++;
    }

    {
        const int left   = 40;
        const int right  = 300;
        const int top    = 70;
        const int bottom = 190;
        const int strings = 6;
        const int frets   = 12;

        vga_fill_rect(left, top, right - left, bottom - top, COLOR_WHITE);
        vga_draw_rect(left, top, right - left, bottom - top, COLOR_BORDER);

        vga_fill_rect(left, top, 4, bottom - top, COLOR_BORDER);

        for (int f = 1; f <= frets; f++) {
            int x = left + (f * (right - left)) / (frets + 1);
            for (int y = top; y < bottom; y++) {
                vga_put_pixel(x, y, COLOR_BORDER);
            }
        }

        for (int s = 0; s < strings; s++) {
            int y = top + ((s + 1) * (bottom - top)) / (strings + 1);

            for (int x = left; x < right; x++) {
                vga_put_pixel(x, y, COLOR_BORDER);
            }

            buf[((5 + s) << 7) + 2] = '6' - s;
        }

        for (int f = 1; f <= frets; f++) {
            if (f == 3 || f == 5 || f == 7 || f == 9 || f == 12) {
                int x0 = left + ((f - 1) * (right - left)) / (frets + 1);
                int x1 = left + (f * (right - left)) / (frets + 1);
                int cx = (x0 + x1) / 2;

                if (f == 12) {
                    int cy1 = top + (bottom - top) / 3;
                    int cy2 = top + 2 * (bottom - top) / 3;
                    vga_fill_rect(cx - 2, cy1 - 2, 4, 4, COLOR_BORDER);
                    vga_fill_rect(cx - 2, cy2 - 2, 4, 4, COLOR_BORDER);
                } else {
                    int cy = (top + bottom) / 2;
                    vga_fill_rect(cx - 2, cy - 2, 4, 4, COLOR_BORDER);
                }
            }
        }
    }
}