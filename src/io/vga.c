#include "io/vga.h"

#include "synth/timbre.h"
#include "io/guitar_view.h"
#include "io/mode.h"
#include "io/piano_view.h"
#include "io/vga_core.h"

void vga_clear(uint16_t color) {
    vga_core_clear_screen(color);
}

void piano_draw_static(void) {
    vga_core_clear_screen(COLOR_BG);
    vga_core_clear_char_buffer();

    if (timbre_get_mode() == TIMBRE_GUITAR) {
        guitar_view_draw_static();
        return;
    }

    piano_view_draw_static();
}

void piano_draw_key(KeyCode key, bool highlighted) {
    if (timbre_get_mode() == TIMBRE_GUITAR) {
        return;
    }
    piano_view_draw_key(key, highlighted);
}

void piano_vga_flush(void) {
    if (timbre_get_mode() == TIMBRE_GUITAR) {
        return;
    }
    piano_view_flush();
}

void piano_handle_key_event(const KeyEvent *event) {
    if (timbre_get_mode() == TIMBRE_GUITAR) {
        return;
    }
    piano_view_handle_key_event(event);
}

void vga_draw_key_label(KeyCode key, bool highlighted) {
    if (timbre_get_mode() == TIMBRE_GUITAR) {
        return;
    }
    piano_view_draw_key_label(key, highlighted);
}

void vga_draw_key_labels(void) {
    if (timbre_get_mode() == TIMBRE_GUITAR) {
        return;
    }
    piano_view_draw_key_labels();
}

void vga_draw_zone_status(void) {
    mode_ui_draw_status();
}