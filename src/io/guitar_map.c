#include "io/guitar_map.h"

static int current_string = 0; /* default = string 6 */

static int string_from_key(KeyCode key) {
    switch (key) {
        case KEY_1: return 0; /* string 6 = low E */
        case KEY_2: return 1; /* string 5 = A */
        case KEY_3: return 2; /* string 4 = D */
        case KEY_4: return 3; /* string 3 = G */
        case KEY_5: return 4; /* string 2 = B */
        case KEY_6: return 5; /* string 1 = high E */
        default:    return -1;
    }
}

static int fret_from_key(KeyCode key) {
    switch (key) {
        case KEY_A: return 0;
        case KEY_W: return 1;
        case KEY_S: return 2;
        case KEY_E: return 3;
        case KEY_D: return 4;
        case KEY_F: return 5;
        case KEY_T: return 6;
        case KEY_G: return 7;
        default:    return -1;
    }
}

/* standard tuning, Hz */
static uint32_t open_string_freq(int string_index) {
    switch (string_index) {
        case 0: return 82;  /* E2 */
        case 1: return 110; /* A2 */
        case 2: return 147; /* D3 */
        case 3: return 196; /* G3 */
        case 4: return 247; /* B3 */
        case 5: return 330; /* E4 */
        default: return 0;
    }
}

/* multiply by semitone ratio approximately using a lookup for frets 0..7 */
static uint32_t freq_for_fret(uint32_t open_hz, int fret) {
    /* rounded multipliers for 12-TET */
    switch (fret) {
        case 0: return open_hz;
        case 1: return (open_hz * 1060u) / 1000u;
        case 2: return (open_hz * 1122u) / 1000u;
        case 3: return (open_hz * 1189u) / 1000u;
        case 4: return (open_hz * 1260u) / 1000u;
        case 5: return (open_hz * 1335u) / 1000u;
        case 6: return (open_hz * 1414u) / 1000u;
        case 7: return (open_hz * 1498u) / 1000u;
        default: return 0;
    }
}

void guitar_map_init(void) {
    current_string = 0;
}

void guitar_map_select_string(KeyCode key) {
    int s = string_from_key(key);
    if (s >= 0) {
        current_string = s;
    }
}

bool guitar_map_is_string_key(KeyCode key) {
    return string_from_key(key) >= 0;
}

bool guitar_map_is_fret_key(KeyCode key) {
    return fret_from_key(key) >= 0;
}

GuitarNote guitar_map_note_from_key(KeyCode key) {
    GuitarNote out;
    out.valid = false;
    out.string_index = current_string;
    out.fret_index = -1;
    out.freq_hz = 0;

    int fret = fret_from_key(key);
    if (fret < 0) {
        return out;
    }

    uint32_t open_hz = open_string_freq(current_string);
    if (open_hz == 0) {
        return out;
    }

    out.valid = true;
    out.fret_index = fret;
    out.freq_hz = freq_for_fret(open_hz, fret);
    return out;
}