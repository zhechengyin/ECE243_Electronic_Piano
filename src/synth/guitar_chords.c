#include "synth/guitar_chords.h"

#include <stdint.h>

/*
 * Guitar chord mode mapping (slightly higher register, demo-friendly):
 *
 * 1 -> Em   = E3, G4, B4
 * 2 -> Am   = A3, C4, E4
 * 3 -> Dm   = D4, F4, A4
 * 4 -> G    = G3, B3, D4
 * 5 -> C    = C4, E4, G4
 * 6 -> F    = F3, A3, C4
 * 7 -> Bb   = Bb3, D4, F4
 * 8 -> Bdim = B3, D4, F4
 */
int guitar_chord_from_key(KeyCode key, uint32_t* out_freqs, int max_count) {
  if (out_freqs == 0 || max_count < GUITAR_CHORD_MAX_NOTES) {
    return 0;
  }

  switch (key) {
    case KEY_1: /* Em */
      out_freqs[0] = 165; /* E3 */
      out_freqs[1] = 392; /* G4 */
      out_freqs[2] = 494; /* B4 */
      return 3;

    case KEY_2: /* Am */
      out_freqs[0] = 220; /* A3 */
      out_freqs[1] = 262; /* C4 */
      out_freqs[2] = 659; /* E4 */
      return 3;

    case KEY_3: /* Dm */
      out_freqs[0] = 294; /* D4 */
      out_freqs[1] = 349; /* F4 */
      out_freqs[2] = 440; /* A4 */
      return 3;

    case KEY_4: /* G */
      out_freqs[0] = 196; /* G3 */
      out_freqs[1] = 247; /* B3 */
      out_freqs[2] = 587; /* D4 */
      return 3;

    case KEY_5: /* C */
      out_freqs[0] = 262; /* C4 */
      out_freqs[1] = 330; /* E4 */
      out_freqs[2] = 392; /* G4 */
      return 3;

    case KEY_6: /* F */
      out_freqs[0] = 175; /* F3 */
      out_freqs[1] = 220; /* A3 */
      out_freqs[2] = 523; /* C4 */
      return 3;

    case KEY_7: /* Bb */
      out_freqs[0] = 233; /* Bb3 */
      out_freqs[1] = 294; /* D4 */
      out_freqs[2] = 698; /* F4 */
      return 3;

    case KEY_8: /* Bdim */
      out_freqs[0] = 247; /* B3 */
      out_freqs[1] = 294; /* D4 */
      out_freqs[2] = 698; /* F4 */
      return 3;

    default:
      return 0;
  }
}