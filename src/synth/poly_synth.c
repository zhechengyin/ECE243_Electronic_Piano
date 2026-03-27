#include "synth/poly_synth.h"

#include <stdint.h>

#include "synth/oscillator.h"
#include "synth/timbre.h"

#define POLY_SYNTH_VOICE_COUNT 32

/* envelope states */
#define ENV_OFF      0
#define ENV_ATTACK   1
#define ENV_DECAY    2
#define ENV_SUSTAIN  3
#define ENV_RELEASE  4

/* fixed envelope range used by the synth engine */
#define ENV_MAX      16384

typedef struct {
  int active;
  KeyCode key;      /* trigger key, not array index */
  uint32_t freq;
  uint32_t phase;
  uint32_t step;
  uint16_t env;
  uint8_t env_state;
  uint8_t sustain_div;
  uint8_t pluck_count;
} Voice;

static Voice voices[POLY_SYNTH_VOICE_COUNT];

static int mix_divisor_from_voice_count(int n) {
  if (n <= 1) return 1;
  if (n == 2) return 2;
  if (n <= 4) return 2;
  if (n <= 8) return 3;
  return 4;
}

static int makeup_gain_from_voice_count(int n) {
  if (n <= 1) return 6;
  if (n == 2) return 5;
  if (n <= 4) return 4;
  if (n <= 8) return 2;
  return 1;
}

static int32_t soft_clip24(int64_t x) {
  const int32_t LIMIT = 0x007FFFFF;
  const int32_t KNEE  = 0x00600000;

  if (x > KNEE) {
    x = KNEE + ((x - KNEE) >> 3);
  } else if (x < -KNEE) {
    x = -KNEE + ((x + KNEE) >> 3);
  }

  if (x > LIMIT) return LIMIT;
  if (x < -0x00800000) return -0x00800000;
  return (int32_t)x;
}

static void voice_clear(int i) {
  voices[i].active = 0;
  voices[i].key = KEY_NONE;
  voices[i].freq = 0;
  voices[i].phase = 0;
  voices[i].step = 0;
  voices[i].env = 0;
  voices[i].env_state = ENV_OFF;
  voices[i].sustain_div = 0;
  voices[i].pluck_count = 0;
}

static void update_envelope(int i, const TimbreSpec* spec) {
  switch (voices[i].env_state) {
    case ENV_ATTACK:
      if (voices[i].env + spec->attack_step >= ENV_MAX) {
        voices[i].env = ENV_MAX;
        voices[i].env_state = ENV_DECAY;
      } else {
        voices[i].env += spec->attack_step;
      }
      break;

    case ENV_DECAY:
      if (voices[i].env > spec->decay_target + spec->decay_step) {
        voices[i].env -= spec->decay_step;
      } else {
        voices[i].env = spec->decay_target;
        voices[i].env_state = ENV_SUSTAIN;
        voices[i].sustain_div = 0;
      }
      break;

    case ENV_SUSTAIN:
      if (voices[i].env > spec->sustain_target) {
        if (spec->sustain_decay_step == 0) {
          voices[i].env = spec->sustain_target;
        } else {
          uint8_t div = spec->sustain_decay_div ? spec->sustain_decay_div : 1;
          voices[i].sustain_div++;

          if (voices[i].sustain_div >= div) {
            voices[i].sustain_div = 0;

            if (voices[i].env > spec->sustain_target + spec->sustain_decay_step) {
              voices[i].env -= spec->sustain_decay_step;
            } else {
              voices[i].env = spec->sustain_target;
            }
          }
        }
      } else {
        voices[i].env = spec->sustain_target;
      }
      break;

    case ENV_RELEASE:
      if (spec->release_step == 0) {
        voice_clear(i);
      } else if (voices[i].env > spec->release_step) {
        voices[i].env -= spec->release_step;
      } else {
        voice_clear(i);
      }
      break;

    case ENV_OFF:
    default:
      voices[i].env = 0;
      break;
  }
}

static int find_free_voice_slot(void) {
  for (int i = 0; i < POLY_SYNTH_VOICE_COUNT; i++) {
    if (!voices[i].active) {
      return i;
    }
  }

  for (int i = 0; i < POLY_SYNTH_VOICE_COUNT; i++) {
    if (voices[i].env_state == ENV_RELEASE) {
      return i;
    }
  }

  return 0;
}

void poly_synth_init(void) {
  osc_init();

  for (int i = 0; i < POLY_SYNTH_VOICE_COUNT; i++) {
    voice_clear(i);
  }
}

void poly_synth_note_on(KeyCode key, uint32_t freq_hz) {
  const TimbreSpec* spec = timbre_get_spec();
  int slot;

  if (key <= KEY_NONE || key >= KEY_COUNT || freq_hz == 0) {
    return;
  }

  slot = find_free_voice_slot();

  voices[slot].active = 1;
  voices[slot].key = key;
  voices[slot].freq = freq_hz;
  voices[slot].phase = 0;
  voices[slot].step = osc_phase_step_from_freq(freq_hz);
  voices[slot].env = 0;
  voices[slot].env_state = ENV_ATTACK;
  voices[slot].sustain_div = 0;
  voices[slot].pluck_count = spec->pluck_samples;
}

void poly_synth_note_off(KeyCode key) {
  if (key <= KEY_NONE || key >= KEY_COUNT) {
    return;
  }

  for (int i = 0; i < POLY_SYNTH_VOICE_COUNT; i++) {
    if (voices[i].active && voices[i].key == key && voices[i].env_state != ENV_OFF) {
      voices[i].env_state = ENV_RELEASE;
    }
  }
}

int poly_synth_active_count(void) {
  int count = 0;

  for (int i = 0; i < POLY_SYNTH_VOICE_COUNT; i++) {
    if (voices[i].active) {
      count++;
    }
  }

  return count;
}

int32_t poly_synth_next_sample(void) {
  const TimbreSpec* spec = timbre_get_spec();
  int64_t mix = 0;
  int ringing_count = 0;

  for (int i = 0; i < POLY_SYNTH_VOICE_COUNT; i++) {
    if (!voices[i].active) {
      continue;
    }

    update_envelope(i, spec);

    if (!voices[i].active) {
      continue;
    }

    voices[i].phase += voices[i].step;

    {
      int64_t raw = timbre_render_raw_sample(voices[i].phase,
                                             voices[i].pluck_count,
                                             spec);

      if (voices[i].pluck_count > 0) {
        voices[i].pluck_count--;
      }

      mix += (raw * voices[i].env) / ENV_MAX;
      ringing_count++;
    }
  }

  if (ringing_count == 0) {
    return 0;
  }

  mix /= mix_divisor_from_voice_count(ringing_count);
  mix *= makeup_gain_from_voice_count(ringing_count);

  return soft_clip24(mix);
}
