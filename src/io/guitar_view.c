#include "io/guitar_view.h"

#include <stdbool.h>

#include "app/key_event.h"
#include "io/vga.h"
#include "io/vga_core.h"

/* ---------- unified geometry ---------- */
#define G_LEFT      48
#define G_TOP       64
#define G_FRET_W    28
#define G_FRETS     8
#define G_WIDTH     (G_FRET_W * G_FRETS)
#define G_RIGHT     (G_LEFT + G_WIDTH)

#define G_BOTTOM    208
#define G_STRINGS   6

#define G_LABEL_ROW 14
#define G_STRING_X  10

static int fret_from_key(KeyCode key) {
    switch (key) {
        case KEY_1: return 0;
        case KEY_2: return 1;
        case KEY_3: return 2;
        case KEY_4: return 3;
        case KEY_5: return 4;
        case KEY_6: return 5;
        case KEY_7: return 6;
        case KEY_8: return 7;
        default:    return -1;
    }
}

static void draw_char_text(int x, int y, const char *s) {
    volatile char *buf = (volatile char *)VGA_CHAR_BUFFER;
    int off = (y << 7) + x;

    while (*s) {
        buf[off++] = *s++;
    }
}

static void clear_char_row(int row) {
    volatile char *buf = (volatile char *)VGA_CHAR_BUFFER;
    for (int x = 0; x < 80; x++) {
        buf[(row << 7) + x] = ' ';
    }
}

static void draw_string_line(int x0, int x1, int y, int thickness, uint16_t color) {
    for (int t = 0; t < thickness; t++) {
        for (int x = x0; x <= x1; x++) {
            vga_put_pixel(x, y + t, color);
        }
    }
}

static void draw_tab_marker(int x, int y, int w, int h, uint16_t fill, uint16_t border) {
    vga_fill_rect(x, y, w, h - 6, fill);
    vga_draw_rect(x, y, w, h - 6, border);

    {
        int cx = x + w / 2;
        for (int dy = 0; dy < 6; dy++) {
            int half = (w / 2) - (dy * (w / 12));
            for (int xx = cx - half; xx <= cx + half; xx++) {
                vga_put_pixel(xx, y + (h - 6) + dy, fill);
            }
            vga_put_pixel(cx - half, y + (h - 6) + dy, border);
            vga_put_pixel(cx + half, y + (h - 6) + dy, border);
        }
    }
}

static int fret_line_x(int i) {
    return G_LEFT + i * G_FRET_W;
}

static int fret_fill_x0(int fret) {
    return fret_line_x(fret) + 2;
}

static int fret_fill_x1(int fret) {
    return fret_line_x(fret + 1) - 2;
}

static int fret_mid_x(int fret) {
    return (fret_fill_x0(fret) + fret_fill_x1(fret)) / 2;
}

static int string_y(int s) {
    return G_TOP + 18 + (s * 20);
}

static void draw_fret_numbers(void) {
    volatile char *buf = (volatile char *)VGA_CHAR_BUFFER;
    int start_char_x = G_LEFT / 4;   /* 48 / 4 = 12 */

    clear_char_row(G_LABEL_ROW);

    for (int i = 0; i < G_FRETS; i++) {
        /* 28 px per fret = 7 text cells per fret */
        int char_x = start_char_x + i * 7 + 3;
        buf[(G_LABEL_ROW << 7) + char_x] = '1' + i;
    }
}

static void draw_string_numbers(void) {
    volatile char *buf = (volatile char *)VGA_CHAR_BUFFER;

    /* clear a small area near the string labels */
    for (int s = 0; s < G_STRINGS; s++) {
        int y = string_y(s);
        int char_y = y / 4;

        if (char_y >= 0 && char_y < 60) {
            buf[(char_y << 7) + G_STRING_X] = ' ';
        }
    }

    for (int s = 0; s < G_STRINGS; s++) {
        int y = string_y(s);
        int char_y = y / 4;   /* directly from the string y */

        if (char_y < 0) {
            char_y = 0;
        }
        if (char_y > 59) {
            char_y = 59;
        }

        buf[(char_y << 7) + G_STRING_X] = '6' - s;
    }
}

static void redraw_fretboard_base(void) {
    vga_fill_rect(0, 0, SCREEN_W, SCREEN_H, 0xDEFB);

    /* board */
    vga_fill_rect(G_LEFT, G_TOP, G_WIDTH, G_BOTTOM - G_TOP, COLOR_WHITE);

    /* only left and right borders */
    for (int y = G_TOP; y < G_BOTTOM; y++) {
        vga_put_pixel(G_LEFT, y, COLOR_BORDER);
        vga_put_pixel(G_LEFT + 1, y, COLOR_BORDER);
        vga_put_pixel(G_RIGHT - 1, y, COLOR_BORDER);
        vga_put_pixel(G_RIGHT - 2, y, COLOR_BORDER);
    }

    /* side strips */
    vga_fill_rect(G_LEFT, G_TOP, 10, G_BOTTOM - G_TOP, 0xC618);
    vga_fill_rect(G_RIGHT - 10, G_TOP, 10, G_BOTTOM - G_TOP, 0xC618);

    /* fret lines exactly on 28 px grid */
    for (int f = 0; f <= G_FRETS; f++) {
        int x = fret_line_x(f);
        for (int y = G_TOP; y < G_BOTTOM; y++) {
            vga_put_pixel(x, y, 0x7BEF);
            vga_put_pixel(x + 1, y, 0x7BEF);
        }
    }

    /* strings */
    for (int s = 0; s < G_STRINGS; s++) {
        int y = string_y(s);
        int thickness = (s < 2) ? 2 : 1;
        draw_string_line(G_LEFT, G_RIGHT, y, thickness, 0x8410);
    }
}

static void draw_fret_highlight(int fret, bool on) {
    int x0, x1, fill_w;
    uint16_t fill;

    if (fret < 0 || fret >= G_FRETS) {
        return;
    }

    x0 = fret_fill_x0(fret);
    x1 = fret_fill_x1(fret);
    fill_w = x1 - x0 + 1;

    if (fill_w <= 0) {
        return;
    }

    fill = on ? 0x07FF : COLOR_WHITE;

    /* fill exactly the inside of the fret column */
    vga_fill_rect(x0, G_TOP + 2, fill_w, G_BOTTOM - G_TOP - 4, fill);

    /* redraw side strips */
    vga_fill_rect(G_LEFT, G_TOP, 10, G_BOTTOM - G_TOP, 0xC618);
    vga_fill_rect(G_RIGHT - 10, G_TOP, 10, G_BOTTOM - G_TOP, 0xC618);

    /* redraw fret lines */
    for (int f = 0; f <= G_FRETS; f++) {
        int x = fret_line_x(f);
        for (int y = G_TOP; y < G_BOTTOM; y++) {
            vga_put_pixel(x, y, 0x7BEF);
            vga_put_pixel(x + 1, y, 0x7BEF);
        }
    }

    /* redraw strings */
    for (int s = 0; s < G_STRINGS; s++) {
        int y = string_y(s);
        int thickness = (s < 2) ? 2 : 1;
        draw_string_line(G_LEFT, G_RIGHT, y, thickness, 0x8410);
    }

    for (int y = G_TOP; y < G_BOTTOM; y++) {
        vga_put_pixel(G_LEFT, y, COLOR_BORDER);
        vga_put_pixel(G_LEFT + 1, y, COLOR_BORDER);
        vga_put_pixel(G_RIGHT - 1, y, COLOR_BORDER);
        vga_put_pixel(G_RIGHT - 2, y, COLOR_BORDER);
    }
}

static void draw_chord_tabs(void) {
    static const char *labels[8] = {"Em", "Am", "Dm", "G", "C", "F", "Bb", "Bdim"};
    const int tab_w = 22;
    const int tab_h = 34;
    const int y = 34;

    /* center one tab over each fret column */
    for (int i = 0; i < G_FRETS; i++) {
        int cx = fret_mid_x(i);
        int x = cx - tab_w / 2;
        uint16_t fill = 0xC618;
        uint16_t border = 0x7BCF;

        if (i == 7) {
            fill = 0x041F;
            border = 0x0010;
        }

        draw_tab_marker(x, y, tab_w, tab_h, fill, border);

        const char *label = labels[i];
        int len = 0;
        int center_px;
        int char_x;

        while (label[len] != '\0') {
            len++;
        }

        /* center based on the tab center, not the left edge */
        center_px = x + tab_w / 2;

        /* convert pixel center to char cell, with rounding */
        char_x = (center_px + 2) / 4;

        /* shift left by half the label width */
        char_x -= len / 2;

        draw_char_text(char_x, 9, label);
    }
}

void guitar_view_draw_static(void) {
    vga_core_clear_char_buffer();
    vga_core_clear_screen(COLOR_BG);

    draw_char_text(2, 1, "MODE: GUITAR");
    draw_char_text(2, 5, "FRETS: 1 2 3 4 5 6 7 8");

    redraw_fretboard_base();
    draw_chord_tabs();
    draw_fret_numbers();
    draw_string_numbers();
}

void guitar_view_handle_key_event(const KeyEvent *event) {
    int fret;

    if (event == 0) {
        return;
    }

    fret = fret_from_key(event->key);
    if (fret < 0) {
        return;
    }

    draw_fret_highlight(fret, event->pressed != 0);
    draw_fret_numbers();
    draw_string_numbers();
}