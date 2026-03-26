#include "synth/notes.h"
#include <stdint.h>

static int current_zone = 3;   /* default = zone 4 on screen */

/*
 * zone 0  : A0, A#0, B0
 * zone 1  : C1  ~ B1
 * zone 2  : C2  ~ B2
 * zone 3  : C3  ~ B3
 * zone 4  : C4  ~ B4
 * zone 5  : C5  ~ B5
 * zone 6  : C6  ~ B6
 * zone 7  : C7  ~ B7
 *
 * Note:
 * With the current 12-key playing layout, this covers 87 playable piano notes:
 * A0, A#0, B0, C1~B7
 *
 * The final piano key C8 (4186 Hz) is NOT mapped here.
 * Also, with SAMPLE_RATE = 8000, C8 is above Nyquist anyway.
 */

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

/* zone 0 special: A0, A#0, B0 */
static const uint32_t zone0_freqs[3] = {
    27,  /* A0  */
    29,  /* A#0 */
    31   /* B0  */
};

/* zones 1..7: full octave Cn..Bn */
static const uint32_t zone_freqs[7][12] = {
    /* zone 1: C1 ~ B1 */
    {  33,   35,   37,   39,   41,   44,   46,   49,   52,   55,   58,   62 },

    /* zone 2: C2 ~ B2 */
    {  65,   69,   73,   78,   82,   87,   93,   98,  104,  110,  117,  123 },

    /* zone 3: C3 ~ B3 */
    { 131,  139,  147,  156,  165,  175,  185,  196,  208,  220,  233,  247 },

    /* zone 4: C4 ~ B4 */
    { 262,  277,  294,  311,  330,  349,  370,  392,  415,  440,  466,  494 },

    /* zone 5: C5 ~ B5 */
    { 523,  554,  587,  622,  659,  698,  740,  784,  831,  880,  932,  988 },

    /* zone 6: C6 ~ B6 */
    { 1047, 1109, 1175, 1245, 1319, 1397, 1480, 1568, 1661, 1760, 1865, 1976 },

    /* zone 7: C7 ~ B7 */
    { 2093, 2217, 2349, 2489, 2637, 2794, 2960, 3136, 3322, 3520, 3729, 3951 }
};

/*
 * Map playable keys to semitone index in one octave:
 *
 * A -> C
 * W -> C#
 * S -> D
 * E -> D#
 * D -> E
 * F -> F
 * T -> F#
 * G -> G
 * Y -> G#
 * H -> A
 * U -> A#
 * J -> B
 */
static int key_to_semitone_index(KeyCode k) {
    switch (k) {
        case KEY_A: return 0;   /* C  */
        case KEY_W: return 1;   /* C# */
        case KEY_S: return 2;   /* D  */
        case KEY_E: return 3;   /* D# */
        case KEY_D: return 4;   /* E  */
        case KEY_F: return 5;   /* F  */
        case KEY_T: return 6;   /* F# */
        case KEY_G: return 7;   /* G  */
        case KEY_Y: return 8;   /* G# */
        case KEY_H: return 9;   /* A  */
        case KEY_U: return 10;  /* A# */
        case KEY_J: return 11;  /* B  */
        default:    return -1;
    }
}

uint32_t note_freq_from_keycode(KeyCode k) {
    int zone = notes_get_zone();

    /* zone 0 only supports A0, A#0, B0 */
    if (zone == 0) {
        switch (k) {
            case KEY_A: return zone0_freqs[0];  /* A0  */
            case KEY_W: return zone0_freqs[1];  /* A#0 */
            case KEY_S: return zone0_freqs[2];  /* B0  */
            default:    return 0;
        }
    }

    /* zones 1..7 support full octave Cn..Bn */
    if (zone >= 1 && zone <= 7) {
        int idx = key_to_semitone_index(k);
        if (idx < 0) {
            return 0;
        }
        return zone_freqs[zone - 1][idx];
    }

    return 0;
}