#include "app/piano_engine.h"

#include "synth/notes.h"
#include "synth/oscillator.h"
#include <stdint.h>

/*
    This module is the "brain" of the piano application.
    It receives key events from the PS/2 input, maps them to musical notes,
    and controls the synthesizer accordingly.
*/

#define PIANO_KEY_COUNT (KEY_L + 1)

static int key_active[PIANO_KEY_COUNT];     // The current status of each key (1=pressed, 0=released)
static uint32_t key_freq[PIANO_KEY_COUNT];
static uint32_t key_phase[PIANO_KEY_COUNT];
static uint32_t key_step[PIANO_KEY_COUNT];
static int active_count = 0;

static int32_t clamp24(int64_t x) {
  if (x > 0x007FFFFF) return 0x007FFFFF;
  if (x < -0x00800000) return -0x00800000;
  return (int32_t)x;
}


void piano_engine_init(void) {
  osc_init();

  for (int i = 0; i < PIANO_KEY_COUNT; i++) {
    key_active[i] = 0;
    key_freq[i] = 0;
    key_phase[i] = 0;
    key_step[i] = 0;
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

  if (f == 0) return;

  if (ev.pressed) {
    if (!key_active[ev.key]) {
      key_active[ev.key] = 1;
      key_freq[ev.key] = f;
      key_phase[ev.key] = 0;
      key_step[ev.key] = osc_phase_step_from_freq(f);
      active_count++;
    }
  }else {
    if (key_active[ev.key]){
      key_active[ev.key] = 0;
      key_freq[ev.key] = 0;
      key_phase[ev.key] = 0;
      key_step[ev.key] = 0;
      active_count--;

      if (active_count < 0) {
        active_count = 0;
      }
    }
  }
}

int32_t piano_engine_next_sample(void) {
  if (active_count == 0){
    return 0;
  }

  int64_t mix = 0;
  for (int i = 1; i < PIANO_KEY_COUNT; i++){
    if (key_active[i]){
      key_phase[i] += key_step[i];
      mix += osc_sample_from_phase(key_phase[i]);
    }
  }
  // Simple normalization
  mix /= active_count;
  return clamp24(mix);
}