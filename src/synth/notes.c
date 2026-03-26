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
    int zone = notes_get_zone();   /* 0..7 */

    /* zone 0 = lowest partial octave: A0, A#0, B0 */
    if (zone == 0) {
        switch (k) {
            case KEY_A: return 27;  // A0
            case KEY_W: return 29;  // A#0 / Bb0
            case KEY_S: return 31;  // B0
            default:    return 0;
        }
    }

    /* other zones = full octave */
    switch (k) {
        case KEY_A: return base_C_for_zone(zone);      // Cn
        case KEY_W: return base_Cs_for_zone(zone);     // C#n
        case KEY_S: return base_D_for_zone(zone);      // Dn
        case KEY_E: return base_Ds_for_zone(zone);     // D#n
        case KEY_D: return base_E_for_zone(zone);      // En
        case KEY_F: return base_F_for_zone(zone);      // Fn
        case KEY_T: return base_Fs_for_zone(zone);     // F#n
        case KEY_G: return base_G_for_zone(zone);      // Gn
        case KEY_Y: return base_Gs_for_zone(zone);     // G#n
        case KEY_H: return base_A_for_zone(zone);      // An
        case KEY_U: return base_As_for_zone(zone);     // A#n
        case KEY_J: return base_B_for_zone(zone);      // Bn
        default:    return 0;
    }
}