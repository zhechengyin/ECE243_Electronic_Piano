#include "io/vga.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "platform/address_map.h"

#define VGA_PIXEL_BUFFER FPGA_PIXEL_BUF_BASE
#define VGA_CHAR_BUFFER  FPGA_CHAR_BASE

#define CHAR_W 80
#define CHAR_H 60

static bool white_down[NUM_WHITE];
static bool black_down[NUM_BLACK];

/* black keys exist after white keys: 0,1,3,4,5,7 */
static const int black_after_white[NUM_BLACK] = {0, 1, 3, 4, 5, 7};

/* ---------- internal helpers ---------- */

static void vga_clear_char_buffer(void);
static int  white_index_from_key(KeyCode key);
static int  black_index_from_key(KeyCode key);

static bool has_black_after_white(int white_idx);
static bool has_black_before_white(int white_idx);

static void draw_white_key(int i, bool pressed);
static void draw_black_key(int i, bool pressed);

static void draw_key_letter_pixels(int x, int y, char c, uint16_t color);
static void draw_white_label_at(int i);
static void draw_black_label_at(int i);
static void draw_labels_on_keys(void);

static void redraw_keyboard(void);
static void redraw_region_for_white(int i);
static void redraw_region_for_black(int i);

/* ---------- low-level VGA ---------- */

static void vga_clear_char_buffer(void) {
    volatile char *char_buf = (volatile char *)VGA_CHAR_BUFFER;

    for (int y = 0; y < CHAR_H; y++) {
        for (int x = 0; x < CHAR_W; x++) {
            char_buf[(y << 7) + x] = ' ';
        }
    }
}

void vga_put_pixel(int x, int y, uint16_t color) {
    if (x < 0 || x >= SCREEN_W || y < 0 || y >= SCREEN_H) {
        return;
    }

    volatile uint16_t *pixel =
        (volatile uint16_t *)(VGA_PIXEL_BUFFER + (y << 10) + (x << 1));
    *pixel = color;
}

void vga_fill_rect(int x, int y, int width, int height, uint16_t color) {
    int x0 = x;
    int y0 = y;
    int x1 = x + width;
    int y1 = y + height;

    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 > SCREEN_W) x1 = SCREEN_W;
    if (y1 > SCREEN_H) y1 = SCREEN_H;

    for (int yy = y0; yy < y1; yy++) {
        for (int xx = x0; xx < x1; xx++) {
            vga_put_pixel(xx, yy, color);
        }
    }
}

void vga_draw_rect(int x, int y, int width, int height, uint16_t color) {
    if (width <= 0 || height <= 0) {
        return;
    }

    for (int xx = x; xx < x + width; xx++) {
        vga_put_pixel(xx, y, color);
        vga_put_pixel(xx, y + height - 1, color);
    }

    for (int yy = y; yy < y + height; yy++) {
        vga_put_pixel(x, yy, color);
        vga_put_pixel(x + width - 1, yy, color);
    }
}

void vga_clear(uint16_t color) {
    vga_fill_rect(0, 0, SCREEN_W, SCREEN_H, color);
}

/* ---------- key mapping ---------- */

static int white_index_from_key(KeyCode key) {
    switch (key) {
        case KEY_A: return 0;
        case KEY_S: return 1;
        case KEY_D: return 2;
        case KEY_F: return 3;
        case KEY_G: return 4;
        case KEY_H: return 5;
        case KEY_J: return 6;
        case KEY_K: return 7;
        case KEY_L: return 8;
        default:    return -1;
    }
}

static int black_index_from_key(KeyCode key) {
    switch (key) {
        case KEY_W: return 0;
        case KEY_E: return 1;
        case KEY_T: return 2;
        case KEY_Y: return 3;
        case KEY_U: return 4;
        case KEY_O: return 5;
        default:    return -1;
    }
}

static bool has_black_after_white(int white_idx) {
    for (int i = 0; i < NUM_BLACK; i++) {
        if (black_after_white[i] == white_idx) {
            return true;
        }
    }
    return false;
}

static bool has_black_before_white(int white_idx) {
    return has_black_after_white(white_idx - 1);
}

/* ---------- piano key drawing ---------- */

static void draw_white_key(int i, bool pressed) {
    int x = KB_X + i * WHITE_W;
    int y = KB_Y;
    uint16_t color = pressed ? COLOR_WHITE_ON : COLOR_WHITE;

    /* top section ends just before the black-key bottom level */
    int top_h = BLACK_H - 1;
    int bot_y = y + top_h;
    int bot_h = WHITE_H - top_h;

    int left_notch  = has_black_before_white(i) ? (BLACK_W / 2) : 0;
    int right_notch = has_black_after_white(i)  ? (BLACK_W / 2) : 0;

    int top_x = x + left_notch;
    int top_w = WHITE_W - left_notch - right_notch;

    /* fill top narrow part */
    if (top_w > 0) {
        vga_fill_rect(top_x, y, top_w, top_h, color);
    }

    /* fill bottom full-width part */
    vga_fill_rect(x, bot_y, WHITE_W, bot_h, color);

    /* outline top section: only top + left + right */
    if (top_w > 0) {
        for (int xx = top_x; xx < top_x + top_w; xx++) {
            vga_put_pixel(xx, y, COLOR_BORDER);
        }
        for (int yy = y; yy < y + top_h; yy++) {
            vga_put_pixel(top_x, yy, COLOR_BORDER);
            vga_put_pixel(top_x + top_w - 1, yy, COLOR_BORDER);
        }
    }

    /* outline bottom section: left + right + bottom only
       DO NOT draw a top border here, or you'll get the long horizontal line */
    for (int yy = bot_y; yy < y + WHITE_H; yy++) {
        vga_put_pixel(x, yy, COLOR_BORDER);
        vga_put_pixel(x + WHITE_W - 1, yy, COLOR_BORDER);
    }
    for (int xx = x; xx < x + WHITE_W; xx++) {
        vga_put_pixel(xx, y + WHITE_H - 1, COLOR_BORDER);
    }

    /* shoulder horizontals */
    if (left_notch > 0) {
        for (int xx = x; xx <= x + left_notch; xx++) {
            vga_put_pixel(xx, bot_y, COLOR_BORDER);
        }
    }
    if (right_notch > 0) {
        for (int xx = x + WHITE_W - right_notch - 1; xx < x + WHITE_W; xx++) {
            vga_put_pixel(xx, bot_y, COLOR_BORDER);
        }
    }
}

static void draw_black_key(int i, bool pressed) {
    int left_white = black_after_white[i];
    int x = KB_X + (left_white + 1) * WHITE_W - (BLACK_W / 2);
    int y = KB_Y;
    uint16_t color = pressed ? COLOR_BLACK_ON : COLOR_BLACK;

    vga_fill_rect(x, y, BLACK_W, BLACK_H, color);

    /* top border */
    for (int xx = x; xx < x + BLACK_W; xx++) {
        vga_put_pixel(xx, y, COLOR_BORDER);
    }

    /* left + right borders */
    for (int yy = y; yy < y + BLACK_H; yy++) {
        vga_put_pixel(x, yy, COLOR_BORDER);
        vga_put_pixel(x + BLACK_W - 1, yy, COLOR_BORDER);
    }

    /* bottom border */
    for (int xx = x; xx < x + BLACK_W; xx++) {
        vga_put_pixel(xx, y + BLACK_H - 1, COLOR_BORDER);
    }
}
/* ---------- 5x7 bitmap font, scaled x2 ---------- */

static void draw_key_letter_pixels(int x, int y, char c, uint16_t color) {
    static const uint8_t FONT_A[7] = {0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11};
    static const uint8_t FONT_D[7] = {0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E};
    static const uint8_t FONT_E[7] = {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F};
    static const uint8_t FONT_F[7] = {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10};
    static const uint8_t FONT_G[7] = {0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0E};
    static const uint8_t FONT_H[7] = {0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11};
    static const uint8_t FONT_J[7] = {0x07, 0x02, 0x02, 0x02, 0x12, 0x12, 0x0C};
    static const uint8_t FONT_K[7] = {0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11};
    static const uint8_t FONT_L[7] = {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F};
    static const uint8_t FONT_O[7] = {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E};
    static const uint8_t FONT_S[7] = {0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E};
    static const uint8_t FONT_T[7] = {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04};
    static const uint8_t FONT_U[7] = {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E};
    static const uint8_t FONT_W[7] = {0x11, 0x11, 0x11, 0x15, 0x15, 0x15, 0x0A};
    static const uint8_t FONT_Y[7] = {0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04};

    const uint8_t *glyph = NULL;
    const int scale = 2;

    switch (c) {
        case 'A': glyph = FONT_A; break;
        case 'D': glyph = FONT_D; break;
        case 'E': glyph = FONT_E; break;
        case 'F': glyph = FONT_F; break;
        case 'G': glyph = FONT_G; break;
        case 'H': glyph = FONT_H; break;
        case 'J': glyph = FONT_J; break;
        case 'K': glyph = FONT_K; break;
        case 'L': glyph = FONT_L; break;
        case 'O': glyph = FONT_O; break;
        case 'S': glyph = FONT_S; break;
        case 'T': glyph = FONT_T; break;
        case 'U': glyph = FONT_U; break;
        case 'W': glyph = FONT_W; break;
        case 'Y': glyph = FONT_Y; break;
        default: return;
    }

    for (int row = 0; row < 7; row++) {
        for (int col = 0; col < 5; col++) {
            if (glyph[row] & (1 << (4 - col))) {
                vga_fill_rect(x + col * scale, y + row * scale, scale, scale, color);
            }
        }
    }
}

static void draw_white_label_at(int i) {
    static const char white_labels[NUM_WHITE] = {'A','S','D','F','G','H','J','K','L'};
    int key_x = KB_X + i * WHITE_W;
    int label_x = key_x + (WHITE_W - 10) / 2;
    int label_y = KB_Y + WHITE_H - 20;

    draw_key_letter_pixels(label_x, label_y, white_labels[i], 0x0000);
}

static void draw_black_label_at(int i) {
    static const char black_labels[NUM_BLACK] = {'W','E','T','Y','U','O'};
    int left_white = black_after_white[i];
    int key_x = KB_X + (left_white + 1) * WHITE_W - (BLACK_W / 2);
    int label_x = key_x + (BLACK_W - 10) / 2;
    int label_y = KB_Y + BLACK_H - 18;

    draw_key_letter_pixels(label_x, label_y, black_labels[i], 0xFFFF);
}

static void draw_labels_on_keys(void) {
    for (int i = 0; i < NUM_WHITE; i++) {
        draw_white_label_at(i);
    }
    for (int i = 0; i < NUM_BLACK; i++) {
        draw_black_label_at(i);
    }
}

/* ---------- redraw logic ---------- */

static void redraw_keyboard(void) {
    vga_fill_rect(KB_X - 2, KB_Y - 2, NUM_WHITE * WHITE_W + 4, WHITE_H + 4, COLOR_BG);

    for (int i = 0; i < NUM_WHITE; i++) {
        draw_white_key(i, white_down[i]);
    }
    for (int i = 0; i < NUM_BLACK; i++) {
        draw_black_key(i, black_down[i]);
    }

    draw_labels_on_keys();
}

static void redraw_region_for_white(int i) {
    int x0 = KB_X + i * WHITE_W;
    int x1 = x0 + WHITE_W;

    for (int b = 0; b < NUM_BLACK; b++) {
        int left = black_after_white[b];
        if (left == i || left + 1 == i) {
            int bx = KB_X + (left + 1) * WHITE_W - (BLACK_W / 2);
            if (bx < x0) x0 = bx;
            if (bx + BLACK_W > x1) x1 = bx + BLACK_W;
        }
    }

    vga_fill_rect(x0, KB_Y, x1 - x0, WHITE_H, COLOR_BG);

    for (int w = 0; w < NUM_WHITE; w++) {
        int wx0 = KB_X + w * WHITE_W;
        int wx1 = wx0 + WHITE_W;
        if (!(wx1 <= x0 || wx0 >= x1)) {
            draw_white_key(w, white_down[w]);
            draw_white_label_at(w);
        }
    }

    for (int b = 0; b < NUM_BLACK; b++) {
        int bx0 = KB_X + (black_after_white[b] + 1) * WHITE_W - (BLACK_W / 2);
        int bx1 = bx0 + BLACK_W;
        if (!(bx1 <= x0 || bx0 >= x1)) {
            draw_black_key(b, black_down[b]);
            draw_black_label_at(b);
        }
    }
}

static void redraw_region_for_black(int i) {
    int x0 = KB_X + (black_after_white[i] + 1) * WHITE_W - (BLACK_W / 2);
    int x1 = x0 + BLACK_W;

    vga_fill_rect(x0, KB_Y, x1 - x0, WHITE_H, COLOR_BG);

    for (int w = 0; w < NUM_WHITE; w++) {
        int wx0 = KB_X + w * WHITE_W;
        int wx1 = wx0 + WHITE_W;
        if (!(wx1 <= x0 || wx0 >= x1)) {
            draw_white_key(w, white_down[w]);
            draw_white_label_at(w);
        }
    }

    draw_black_key(i, black_down[i]);
    draw_black_label_at(i);
}

/* ---------- public API ---------- */

void piano_draw_static(void) {
    for (int i = 0; i < NUM_WHITE; i++) {
        white_down[i] = false;
    }
    for (int i = 0; i < NUM_BLACK; i++) {
        black_down[i] = false;
    }

    vga_clear(COLOR_BG);
    vga_clear_char_buffer();
    redraw_keyboard();
}

void piano_draw_key(KeyCode key, bool highlighted) {
    int idx = white_index_from_key(key);
    if (idx >= 0) {
        if (white_down[idx] != highlighted) {
            white_down[idx] = highlighted;
            redraw_region_for_white(idx);
        }
        return;
    }

    idx = black_index_from_key(key);
    if (idx >= 0) {
        if (black_down[idx] != highlighted) {
            black_down[idx] = highlighted;
            redraw_region_for_black(idx);
        }
    }
}

void piano_handle_key_event(const KeyEvent *event) {
    if (event == NULL) {
        return;
    }

    piano_draw_key(event->key, event->pressed != 0);
}

/* compatibility stubs */

void vga_draw_letter(int x, int y, char c, uint16_t color) {
    draw_key_letter_pixels(x, y, c, color);
}

void vga_draw_key_label(KeyCode key, bool highlighted) {
    (void)highlighted;

    int idx = white_index_from_key(key);
    if (idx >= 0) {
        draw_white_label_at(idx);
        return;
    }

    idx = black_index_from_key(key);
    if (idx >= 0) {
        draw_black_label_at(idx);
    }
}

void vga_draw_key_labels(void) {
    draw_labels_on_keys();
}