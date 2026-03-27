#include "app/piano_engine.h"

#include <stdint.h>
#include <string.h>

#include "synth/guitar_chords.h"
#include "synth/notes.h"
#include "synth/poly_synth.h"
#include "synth/timbre.h"

static uint8_t key_is_down[KEY_COUNT];

static int is_guitar_number_key(KeyCode key) {
  return (key >= KEY_1 && key <= KEY_8);
}

void piano_engine_init(void) {
  memset(key_is_down, 0, sizeof(key_is_down));
  poly_synth_init();
}

void piano_engine_all_notes_off(void) {
  for (int k = 1; k < KEY_COUNT; k++) {
    poly_synth_note_off((KeyCode)k);
    key_is_down[k] = 0;
  }
}

void piano_engine_on_key_event(KeyEvent ev) {
  if (ev.key <= KEY_NONE || ev.key >= KEY_COUNT) {
    return;
  }

  if (ev.pressed) {
    /* Ignore typematic repeats while a key is already held. */
    if (key_is_down[ev.key]) {
      return;
    }
    key_is_down[ev.key] = 1;
  } else {
    /* Ignore stray releases. */
    if (!key_is_down[ev.key]) {
      return;
    }
    key_is_down[ev.key] = 0;
  }

  if (timbre_get_mode() == TIMBRE_GUITAR) {
    if (!is_guitar_number_key(ev.key)) {
      return;
    }

    if (ev.pressed) {
      uint32_t freqs[GUITAR_CHORD_MAX_NOTES];
      int chord_count = guitar_chord_from_key(ev.key, freqs, GUITAR_CHORD_MAX_NOTES);

      for (int i = 0; i < chord_count; i++) {
        poly_synth_note_on(ev.key, freqs[i]);
      }
    } else {
      poly_synth_note_off(ev.key);
    }

    return;
  }

  {
    uint32_t freq = note_freq_from_keycode(ev.key);

    if (freq == 0) {
      return;
    }

    if (ev.pressed) {
      poly_synth_note_on(ev.key, freq);
    } else {
      poly_synth_note_off(ev.key);
    }
  }
}

int32_t piano_engine_next_sample(void) {
  return poly_synth_next_sample();
}
