#include "synth/notes.h"
#include <stdint.h>

static int current_zone = 3;   /* default = zone 4 on screen */

void notes_set_zone(int z) {
    if (z >= 0 && z < 8) {
        current_zone = z;
    }
}

int notes_get_zone(void) {
    return current_zone;
}

int notes_get_zone_count(void) {
    return 8;
}

static uint32_t apply_zone_shift(uint32_t base) {
    switch (current_zone) {
        case 0: return base / 8;  /* very low */
        case 1: return base / 4;
        case 2: return base / 2;
        case 3: return base;      /* center */
        case 4: return base * 2;
        case 5: return base * 4;
        case 6: return base * 8;
        case 7: return base * 16; /* very high */
        default: return base;
    }
}

/*
    White keys:
    A S D F G H J K L
    -> C4 D4 E4 F4 G4 A4 B4 C5 D5

    Black keys:
    W E T Y U O
    -> C#4 D#4 F#4 G#4 A#4 C#5
*/

uint32_t note_freq_from_keycode(KeyCode k) {
    uint32_t base = 0;
    switch (k) {
        case KEY_A: return 261; // C4
        case KEY_W: return 277; // C#4
        case KEY_S: return 294; // D4
        case KEY_E: return 311; // D#4
        case KEY_D: return 329; // E4

        case KEY_F: return 349; // F4
        case KEY_T: return 370; // F#4
        case KEY_G: return 392; // G4
        case KEY_Y: return 415; // G#4
        case KEY_H: return 440; // A4
        case KEY_U: return 466; // A#4
        case KEY_J: return 493; // B4

        case KEY_K: return 523; // C5
        case KEY_O: return 554; // C#5
        case KEY_L: return 587; // D5

        default:
            return 0;
    }

    // APPLY ZONE SHIFT
    if (current_zone == 0) {
        return base / 2;   // LOW octave
    } else if (current_zone == 2) {
        return base * 2;   // HIGH octave
    }
    return base;
}