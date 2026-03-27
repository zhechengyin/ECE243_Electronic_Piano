#ifndef SYNTH_GUITAR_CHORDS_H
#define SYNTH_GUITAR_CHORDS_H

#include <stdint.h>

#include "app/key_event.h"

#define GUITAR_CHORD_MAX_NOTES 3

/*
 * Returns the number of notes in the requested chord.
 *
 * key       : KEY_1 .. KEY_8
 * out_freqs : caller-provided array with room for at least max_count entries
 * max_count : capacity of out_freqs
 *
 * Returns 0 if the key is not a supported guitar chord key.
 */
int guitar_chord_from_key(KeyCode key, uint32_t* out_freqs, int max_count);

#endif
