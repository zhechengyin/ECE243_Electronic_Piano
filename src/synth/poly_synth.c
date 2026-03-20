#include "synth/poly_synth.h"

#include <stdint.h>

#include "synth/oscillator.h"

#define POLY_SYNTH_VOICE_COUNT (KEY_L + 1)

/* envelope states */
#define ENV_OFF      0
#define ENV_ATTACK   1
#define ENV_DECAY    2
#define ENV_SUSTAIN  3
#define ENV_RELEASE  4

/* envelope parameters */
#define ENV_MAX          4096
#define ENV_SUSTAIN_LVL  2200

#define ENV_ATTACK_STEP   512
#define ENV_DECAY_STEP     12
#define ENV_RELEASE_STEP   24

typedef struct {
  int active;          /* 1 = this voice is still ringing */
  KeyCode key;
  uint32_t freq;
  uint32_t phase;
  uint32_t step;
  uint16_t env;
  uint8_t env_state;
} Voice;

static Voice voices[POLY_SYNTH_VOICE_COUNT];

static int mix_divisor_from_voice_count(int n) {
  if (n <= 1) return 1;
  if (n == 2) return 2;
  if (n <= 4) return 2;
  return 4;
}

static int makeup_gain_from_voice_count(int n) {
  if (n <= 1) return 9;
  if (n == 2) return 8;
  if (n <= 4) return 6;
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
}

static void update_envelope(int i) {
  switch (voices[i].env_state) {
    case ENV_ATTACK:
      if (voices[i].env + ENV_ATTACK_STEP >= ENV_MAX) {
        voices[i].env = ENV_MAX;
        voices[i].env_state = ENV_DECAY;
      } else {
        voices[i].env += ENV_ATTACK_STEP;
      }
      break;

    case ENV_DECAY:
      if (voices[i].env > ENV_SUSTAIN_LVL + ENV_DECAY_STEP) {
        voices[i].env -= ENV_DECAY_STEP;
      } else {
        voices[i].env = ENV_SUSTAIN_LVL;
        voices[i].env_state = ENV_SUSTAIN;
      }
      break;

    case ENV_SUSTAIN:
      /* hold at sustain level until note_off moves it to release */
      voices[i].env = ENV_SUSTAIN_LVL;
      break;

    case ENV_RELEASE:
      if (voices[i].env > ENV_RELEASE_STEP) {
        voices[i].env -= ENV_RELEASE_STEP;
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

void poly_synth_init(void) {
  osc_init();

  for (int i = 0; i < POLY_SYNTH_VOICE_COUNT; i++) {
    voice_clear(i);
  }
}

void poly_synth_note_on(KeyCode key, uint32_t freq_hz) {
  if (key <= KEY_NONE || key >= POLY_SYNTH_VOICE_COUNT) {
    return;
  }

  voices[key].active = 1;
  voices[key].key = key;
  voices[key].freq = freq_hz;
  voices[key].phase = 0;  /* piano-like retrigger */
  voices[key].step = osc_phase_step_from_freq(freq_hz);
  voices[key].env = 0;
  voices[key].env_state = ENV_ATTACK;
}

void poly_synth_note_off(KeyCode key) {
  if (key <= KEY_NONE || key >= POLY_SYNTH_VOICE_COUNT) {
    return;
  }

  if (voices[key].active && voices[key].env_state != ENV_OFF) {
    voices[key].env_state = ENV_RELEASE;
  }
}

int poly_synth_active_count(void) {
  int count = 0;

  for (int i = 1; i < POLY_SYNTH_VOICE_COUNT; i++) {
    if (voices[i].active) {
      count++;
    }
  }

  return count;
}

int32_t poly_synth_next_sample(void) {
  int64_t mix = 0;
  int ringing_count = 0;

  for (int i = 1; i < POLY_SYNTH_VOICE_COUNT; i++) {
    if (!voices[i].active) {
      continue;
    }

    update_envelope(i);

    if (!voices[i].active) {
      continue;
    }

    voices[i].phase += voices[i].step;

    int32_t raw = osc_sample_from_phase(voices[i].phase);
    int64_t voiced = ((int64_t)raw * voices[i].env) / ENV_MAX;

    mix += voiced;
    ringing_count++;
  }

  if (ringing_count == 0) {
    return 0;
  }

  mix /= mix_divisor_from_voice_count(ringing_count);
  mix *= makeup_gain_from_voice_count(ringing_count);

  return soft_clip24(mix);
}