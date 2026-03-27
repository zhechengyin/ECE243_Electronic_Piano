#include "io/guitar_view.h"

#include "io/vga.h"
#include "io/vga_core.h"

/* layout */
#define G_LEFT    62
#define G_RIGHT   296
#define G_TOP     64
#define G_BOTTOM  208
#define G_FRETS   8
#define G_STRINGS 6

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

static int fret_x(int fret_line) {
    return G_LEFT + (fret_line * (G_RIGHT - G_LEFT)) / G_FRETS;
}

static int fret_mid_x(int fret) {
    int x0 = fret_x(fret);
    int x1 = fret_x(fret + 1);
    return (x0 + x1) / 2;
}

static int string_y(int s) {
    return G_TOP + 18 + (s * 20);
}

static void draw_fret_numbers(void) {
    volatile char *buf = (volatile char *)VGA_CHAR_BUFFER;

    /* clear the row used for fret labels */
    for (int x = 0; x < 80; x++) {
        buf[(14 << 7) + x] = ' ';
    }

    for (int i = 0; i < 8; i++) {
        int x0 = fret_x(i);
        int x1 = fret_x(i + 1);

        /* center of the fret column in pixel space */
        int mid_x = (x0 + x1) / 2;

        /*
         * Character buffer is 80x60 over 320x240,
         * so one text cell is 4 pixels wide.
         *
         * Subtract 2 pixels before dividing so the 1-char label
         * is centered on the fret column instead of starting at mid_x.
         */
        int char_x = (mid_x - 4) / 4;

        /* move one more cell left to visually center better */
        if (((x1 - x0) / 2) > 0) {
            char_x -= 0;
        }

        if (char_x < 0) {
            char_x = 0;
        }
        if (char_x > 79) {
            char_x = 79;
        }

        buf[(14 << 7) + char_x] = '1' + i;
    }
}
static void draw_string_numbers(void) {
    volatile char *buf = (volatile char *)VGA_CHAR_BUFFER;

    for (int s = 0; s < G_STRINGS; s++) {
        int px_y = string_y(s) - 7;   /* center 14px glyph on string */
        int char_y = px_y / 4;
        int char_x = 10;
        buf[(char_y << 7) + char_x] = '6' - s;
    }
}

static void redraw_fretboard_base(void) {
    vga_fill_rect(0, 0, SCREEN_W, SCREEN_H, 0xDEFB);

    /* neutral board */
    vga_fill_rect(G_LEFT, G_TOP, G_RIGHT - G_LEFT, G_BOTTOM - G_TOP, COLOR_WHITE);
    vga_draw_rect(G_LEFT, G_TOP, G_RIGHT - G_LEFT, G_BOTTOM - G_TOP, COLOR_BORDER);

    /* side strips */
    vga_fill_rect(G_LEFT, G_TOP, 10, G_BOTTOM - G_TOP, 0xC618);
    vga_fill_rect(G_RIGHT - 10, G_TOP, 10, G_BOTTOM - G_TOP, 0xC618);

    /* fret lines */
    for (int f = 0; f <= G_FRETS; f++) {
        int x = fret_x(f);
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
    if (fret < 0 || fret >= G_FRETS) {
        return;
    }

    /* fret lines are drawn at x and x+1, so use the inner edges */
    int left_line_x  = fret_x(fret);
    int right_line_x = fret_x(fret + 1);

    int fill_x = left_line_x + 2;
    int fill_w = (right_line_x - 1) - fill_x;

    if (fill_w <= 0) {
        return;
    }

    uint16_t fill = on ? 0x07FF : COLOR_WHITE;

    /* redraw only the inside of the selected fret column */
    vga_fill_rect(fill_x, G_TOP + 2, fill_w, G_BOTTOM - G_TOP - 4, fill);

    /* redraw side strips */
    vga_fill_rect(G_LEFT, G_TOP, 10, G_BOTTOM - G_TOP, 0xC618);
    vga_fill_rect(G_RIGHT - 10, G_TOP, 10, G_BOTTOM - G_TOP, 0xC618);

    /* redraw fret lines */
    for (int f = 0; f <= G_FRETS; f++) {
        int x = fret_x(f);
        for (int y = G_TOP; y < G_BOTTOM; y++) {
            vga_put_pixel(x, y, 0x7BEF);
            vga_put_pixel(x + 1, y, 0x7BEF);
        }
    }

    /* redraw strings on top */
    for (int s = 0; s < G_STRINGS; s++) {
        int y = string_y(s);
        int thickness = (s < 2) ? 2 : 1;
        draw_string_line(G_LEFT, G_RIGHT, y, thickness, 0x8410);
    }

    /* redraw border */
    vga_draw_rect(G_LEFT, G_TOP, G_RIGHT - G_LEFT, G_BOTTOM - G_TOP, COLOR_BORDER);
}

static void draw_chord_tabs(void) {
    static const char *labels[8] = {"Em", "Am", "Dm", "G", "C", "F", "Bb", "Bdim"};

    const int tab_w = 22;
    const int tab_h = 34;
    const int gap   = 6;
    const int start_x = 78;
    const int y = 34;

    for (int i = 0; i < 8; i++) {
        int x = start_x + i * (tab_w + gap);

        uint16_t fill = 0xC618;
        uint16_t border = 0x7BCF;

        if (i == 7) {
            fill = 0x041F;
            border = 0x0010;
        }

        draw_tab_marker(x, y, tab_w, tab_h, fill, border);

        if (i <= 5) {
            draw_char_text((x / 4) + 1, 9, labels[i]);
        } else if (i == 6) {
            draw_char_text((x / 4) + 1, 9, "Bb");
        } else {
            draw_char_text((x / 4), 9, "Bdim");
        }
    }
}

void guitar_view_draw_static(void) {
    vga_core_clear_char_buffer();
    vga_core_clear_screen(COLOR_BG);

    draw_char_text(2, 1, "MODE: GUITAR");
    draw_char_text(2, 3, "OPEN: E2 A2 D3 G3 B3 E4");
    draw_char_text(2, 4, "RANGE: E2 - E6");
    draw_char_text(2, 5, "FRETS: 1 2 3 4 5 6 7 8");

    redraw_fretboard_base();
    draw_chord_tabs();
    draw_fret_numbers();
    draw_string_numbers();
}

void guitar_view_handle_key_event(const KeyEvent *event) {
    if (event == 0) {
        return;
    }

    int fret = fret_from_key(event->key);
    if (fret < 0) {
        return;
    }

    draw_fret_highlight(fret, event->pressed != 0);
    draw_fret_numbers();
    draw_string_numbers();
}