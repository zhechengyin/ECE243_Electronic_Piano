#include "io/piano_view.h"

#include <stdbool.h>
#include <stdint.h>

#include "io/vga.h"
#include "synth/notes.h"
#include "io/vga_core.h"

static bool white_down[NUM_WHITE];
static bool black_down[NUM_BLACK];
static bool dirty_white[NUM_WHITE];
static bool dirty_black[NUM_BLACK];
static volatile bool vga_dirty = false;

static int current_white_count(void) {
    return (notes_get_zone() == 0) ? 2 : 7;
}

static int current_black_count(void) {
    return (notes_get_zone() == 0) ? 1 : 5;
}

static int current_keyboard_x(void) {
    int width = current_white_count() * WHITE_W;
    return (SCREEN_W - width) / 2;
}

static int current_black_after_white(int i) {
    static const int low_partial[1] = {0};
    static const int full_octave[5] = {0, 1, 3, 4, 5};

    if (notes_get_zone() == 0) {
        return low_partial[i];
    }
    return full_octave[i];
}

static int white_index_from_key(KeyCode key) {
    switch (key) {
        case KEY_A: return 0;
        case KEY_S: return 1;
        case KEY_D: return 2;
        case KEY_F: return 3;
        case KEY_G: return 4;
        case KEY_H: return 5;
        case KEY_J: return 6;
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
        default:    return -1;
    }
}

static bool has_black_after_white(int white_idx) {
    for (int i = 0; i < current_black_count(); i++) {
        if (current_black_after_white(i) == white_idx) {
            return true;
        }
    }
    return false;
}

static bool has_black_before_white(int white_idx) {
    return has_black_after_white(white_idx - 1);
}

static void draw_white_key(int i, bool pressed) {
    int x = current_keyboard_x() + i * WHITE_W;
    int y = KB_Y;
    uint16_t color = pressed ? COLOR_WHITE_ON : COLOR_WHITE;

    int top_h = BLACK_H - 1;
    int bot_y = y + top_h;
    int bot_h = WHITE_H - top_h;

    int left_notch  = has_black_before_white(i) ? (BLACK_W / 2) : 0;
    int right_notch = has_black_after_white(i)  ? (BLACK_W / 2) : 0;

    int top_x = x + left_notch;
    int top_w = WHITE_W - left_notch - right_notch;

    if (top_w > 0) {
        vga_fill_rect(top_x, y, top_w, top_h, color);
    }
    vga_fill_rect(x, bot_y, WHITE_W, bot_h, color);

    if (top_w > 0) {
        for (int xx = top_x; xx < top_x + top_w; xx++) {
            vga_put_pixel(xx, y, COLOR_BORDER);
        }
        for (int yy = y; yy < y + top_h; yy++) {
            vga_put_pixel(top_x, yy, COLOR_BORDER);
            vga_put_pixel(top_x + top_w - 1, yy, COLOR_BORDER);
        }
    }

    for (int yy = bot_y; yy < y + WHITE_H; yy++) {
        vga_put_pixel(x, yy, COLOR_BORDER);
        vga_put_pixel(x + WHITE_W - 1, yy, COLOR_BORDER);
    }
    for (int xx = x; xx < x + WHITE_W; xx++) {
        vga_put_pixel(xx, y + WHITE_H - 1, COLOR_BORDER);
    }

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
    int left_white = current_black_after_white(i);
    int x = current_keyboard_x() + (left_white + 1) * WHITE_W - (BLACK_W / 2);
    int y = KB_Y;
    uint16_t color = pressed ? COLOR_BLACK_ON : COLOR_BLACK;

    vga_fill_rect(x, y, BLACK_W, BLACK_H, color);

    for (int xx = x; xx < x + BLACK_W; xx++) {
        vga_put_pixel(xx, y, COLOR_BORDER);
    }
    for (int yy = y; yy < y + BLACK_H; yy++) {
        vga_put_pixel(x, yy, COLOR_BORDER);
        vga_put_pixel(x + BLACK_W - 1, yy, COLOR_BORDER);
    }
    for (int xx = x; xx < x + BLACK_W; xx++) {
        vga_put_pixel(xx, y + BLACK_H - 1, COLOR_BORDER);
    }
}

static void draw_white_label_at(int i) {
    int key_x = current_keyboard_x() + i * WHITE_W;
    int label_x = key_x + (WHITE_W - 10) / 2;
    int label_y = KB_Y + WHITE_H - 20;

    if (notes_get_zone() == 0) {
        static const char low_white[2] = {'A', 'S'};
        vga_draw_letter(label_x, label_y, low_white[i], 0x0000);
    } else {
        static const char full_white[7] = {'A', 'S', 'D', 'F', 'G', 'H', 'J'};
        vga_draw_letter(label_x, label_y, full_white[i], 0x0000);
    }
}

static void draw_black_label_at(int i) {
    int left_white = current_black_after_white(i);
    int key_x = current_keyboard_x() + (left_white + 1) * WHITE_W - (BLACK_W / 2);
    int label_x = key_x + (BLACK_W - 10) / 2;
    int label_y = KB_Y + BLACK_H - 18;

    if (notes_get_zone() == 0) {
        static const char low_black[1] = {'W'};
        vga_draw_letter(label_x, label_y, low_black[i], 0xFFFF);
    } else {
        static const char full_black[5] = {'W', 'E', 'T', 'Y', 'U'};
        vga_draw_letter(label_x, label_y, full_black[i], 0xFFFF);
    }
}

static void draw_labels_on_keys(void) {
    for (int i = 0; i < current_white_count(); i++) {
        draw_white_label_at(i);
    }
    for (int i = 0; i < current_black_count(); i++) {
        draw_black_label_at(i);
    }
}

static void redraw_keyboard(void) {
    int kb_x = current_keyboard_x();
    int kb_w = current_white_count() * WHITE_W;

    vga_fill_rect(kb_x - 2, KB_Y - 2, kb_w + 4, WHITE_H + 4, COLOR_BG);

    for (int i = 0; i < current_white_count(); i++) {
        draw_white_key(i, white_down[i]);
    }
    for (int i = 0; i < current_black_count(); i++) {
        draw_black_key(i, black_down[i]);
    }

    draw_labels_on_keys();
}

static void redraw_region_for_white(int i) {
    if (i < 0 || i >= current_white_count()) return;

    draw_white_key(i, white_down[i]);
    draw_white_label_at(i);

    for (int b = 0; b < current_black_count(); b++) {
        int left = current_black_after_white(b);
        if (left == i || left + 1 == i) {
            draw_black_key(b, black_down[b]);
            draw_black_label_at(b);
        }
    }
}

static void redraw_region_for_black(int i) {
    if (i < 0 || i >= current_black_count()) return;

    draw_black_key(i, black_down[i]);
    draw_black_label_at(i);
}

void piano_view_reset(void) {
    for (int i = 0; i < NUM_WHITE; i++) {
        white_down[i] = false;
        dirty_white[i] = false;
    }
    for (int i = 0; i < NUM_BLACK; i++) {
        black_down[i] = false;
        dirty_black[i] = false;
    }
    vga_dirty = false;
}

void piano_view_draw_static(void) {
    piano_view_reset();
    redraw_keyboard();
}

void piano_view_draw_key(KeyCode key, bool highlighted) {
    int idx = white_index_from_key(key);
    if (idx >= 0) {
        if (idx >= current_white_count()) return;
        if (white_down[idx] != highlighted) {
            white_down[idx] = highlighted;
            dirty_white[idx] = true;
            vga_dirty = true;
        }
        return;
    }

    idx = black_index_from_key(key);
    if (idx >= 0) {
        if (idx >= current_black_count()) return;
        if (black_down[idx] != highlighted) {
            black_down[idx] = highlighted;
            dirty_black[idx] = true;
            vga_dirty = true;
        }
    }
}

void piano_view_flush(void) {
    if (!vga_dirty) return;

    for (int i = 0; i < current_white_count(); i++) {
        if (dirty_white[i]) {
            redraw_region_for_white(i);
            dirty_white[i] = false;
        }
    }

    for (int i = 0; i < current_black_count(); i++) {
        if (dirty_black[i]) {
            redraw_region_for_black(i);
            dirty_black[i] = false;
        }
    }

    vga_dirty = false;
}

void piano_view_handle_key_event(const KeyEvent *event) {
    if (event == 0) return;
    piano_view_draw_key(event->key, event->pressed != 0);
}

void piano_view_draw_key_label(KeyCode key, bool highlighted) {
    (void)highlighted;

    int idx = white_index_from_key(key);
    if (idx >= 0 && idx < current_white_count()) {
        draw_white_label_at(idx);
        return;
    }

    idx = black_index_from_key(key);
    if (idx >= 0 && idx < current_black_count()) {
        draw_black_label_at(idx);
    }
}

void piano_view_draw_key_labels(void) {
    draw_labels_on_keys();
}