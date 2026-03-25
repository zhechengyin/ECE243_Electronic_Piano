#include "app/piano_engine.h"

#include <stdint.h>

#include "synth/notes.h"
#include "synth/poly_synth.h"

#define PIANO_KEY_COUNT KEY_COUNT


/*
    piano_engine:
    - maintain which keys are currently physically pressed
    - forward note on/off events to poly_synth
    - keep main.c interface unchanged
*/

static int key_active[PIANO_KEY_COUNT];
static int active_count = 0;

void piano_engine_init(void) {
  poly_synth_init();

  for (int i = 0; i < PIANO_KEY_COUNT; i++) {
    key_active[i] = 0;
  }

  active_count = 0;
}

int piano_engine_active_count(void) {
  return active_count;
}

int piano_engine_is_key_active(KeyCode key) {
  if (key < 0 || key >= PIANO_KEY_COUNT) {
    return 0;
  }
  return key_active[key];
}

void piano_engine_on_key_event(KeyEvent ev) {
  if (ev.key <= KEY_NONE || ev.key >= PIANO_KEY_COUNT) {
    return;
  }

  uint32_t f = note_freq_from_keycode(ev.key);
  if (f == 0) {
    return;
  }

  if (ev.pressed) {
    /* avoid repeated make codes re-triggering everything */
    if (!key_active[ev.key]) {
      key_active[ev.key] = 1;
      active_count++;
      poly_synth_note_on(ev.key, f);
    }
  } else {
    if (key_active[ev.key]) {
      key_active[ev.key] = 0;
      active_count--;

      if (active_count < 0) {
        active_count = 0;
      }

      poly_synth_note_off(ev.key);
    }
  }
}

int32_t piano_engine_next_sample(void) {
  return poly_synth_next_sample();
}