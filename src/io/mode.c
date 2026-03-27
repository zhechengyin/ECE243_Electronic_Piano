#include "io/mode.h"

#include "io/vga.h"
#include "synth/notes.h"
#include "synth/timbre.h"
#include "io/vga_core.h"

void mode_ui_draw_status(void) {
    volatile char *buf = (volatile char *)VGA_CHAR_BUFFER;
    TimbreMode mode = timbre_get_mode();

    for (int y = 0; y < 2; y++) {
        for (int x = 0; x < 80; x++) {
            buf[(y << 7) + x] = ' ';
        }
    }

    {
        const char *mode_name = "PIANO";
        if (mode == TIMBRE_ORGAN) {
            mode_name = "ORGAN";
        } else if (mode == TIMBRE_GUITAR) {
            mode_name = "GUITAR";
        }

        int off = (0 << 7) + 2;
        const char *label = "MODE: ";

        while (*label) {
            buf[off++] = *label++;
        }
        while (*mode_name) {
            buf[off++] = *mode_name++;
        }
    }

    if (mode == TIMBRE_GUITAR) {
        return;
    }

    {
        const char *title = "OCTAVE ZONES";
        int len = 12;
        int start_x = (CHAR_W - len) / 2;
        int off = (0 << 7) + start_x;

        while (*title) {
            buf[off++] = *title++;
        }
    }

    {
        int zone = notes_get_zone();
        const int box_w = 26;
        const int box_h = 18;
        const int gap   = 4;
        const int total_w = 8 * box_w + 7 * gap;
        const int start_x = (SCREEN_W - total_w) / 2;
        const int y = 12;

        vga_fill_rect(0, 10, SCREEN_W, 24, COLOR_BG);

        for (int i = 0; i < 8; i++) {
            int x = start_x + i * (box_w + gap);
            bool selected = (i == zone);

            uint16_t fill   = selected ? COLOR_WHITE_ON : COLOR_WHITE;
            uint16_t border = COLOR_BORDER;
            uint16_t text   = COLOR_BORDER;

            vga_fill_rect(x, y, box_w, box_h, fill);
            vga_draw_rect(x, y, box_w, box_h, border);

            int num_x = x + (box_w - 10) / 2;
            int num_y = y + (box_h - 14) / 2;
            vga_draw_letter(num_x, num_y, '1' + i, text);
        }
    }
}