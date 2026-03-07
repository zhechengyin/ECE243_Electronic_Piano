#include "synth/notes.h"
#include <stdint.h>

/*
    Map A S D F G H J K L to a one-octave-ish scale starting at C4
    C4 D4 E4 F4 G4 A4 B4 C5 D5
*/

uint32_t note_freq_from_keycode(KeyCode k) {
    switch (k) {
        case KEY_A:
            return 261; // C4
        case KEY_S:
            return 294; // D4
        case KEY_D:
            return 329; // E4
        case KEY_F:
            return 349; // F4
        case KEY_G:
            return 392; // G4
        case KEY_H:
            return 440; // A4
        case KEY_J:
            return 493; // B4
        case KEY_K:
            return 523; // C5
        case KEY_L:
            return 587; // D5
        default:
            return 0;
    }
}