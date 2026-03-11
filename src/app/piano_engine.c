#include "app/piano_engine.h"

#include "synth/notes.h"
#include "synth/synth.h"

/*
    This module is the "brain" of the piano application.
    It receives key events from the PS/2 input, maps them to musical notes,
    and controls the synthesizer accordingly.
*/

static int current_on = 0;
static KeyCode current_key = KEY_NONE;
static uint32_t current_freq = 0;

void piano_engine_init(void) {
  synth_init();
  current_on = 0;
  current_key = KEY_NONE;
  current_freq = 0;
}

void piano_engine_on_key_event(KeyEvent ev) {
  uint32_t f = note_freq_from_keycode(ev.key);

  if (f == 0) return;

  if (ev.pressed) {
    current_on = 1;
    current_key = ev.key;
    current_freq = f;
    synth_set_note(current_on, current_freq);
  } else {
    if (ev.key == current_key) {
      current_on = 0;
      current_key = KEY_NONE;
      synth_set_note(current_on, current_freq);
    }
  }
}