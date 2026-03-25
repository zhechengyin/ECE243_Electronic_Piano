#include "synth/timbre.h"

#include <stdint.h>

#include "synth/oscillator.h"

/* default mode for now */
static TimbreMode current_mode = TIMBRE_GUITAR;

static const TimbreSpec timbre_specs[] = {
    [TIMBRE_PIANO] = {
        .decay_target = 6500,
        .sustain_target = 1400,
        .attack_step = 128,
        .decay_step = 2,
        .sustain_decay_step = 1,
        .release_step = 3,
        .sustain_decay_div = 6,
        .pluck_samples = 0,

        .h2_shift = 2, /* 1/4 */
        .h3_shift = 3, /* 1/8 */
        .h4_shift = 5, /* 1/32 */

        .pluck_h3_shift = 0,
        .pluck_h4_shift = 0,
    },

    [TIMBRE_ORGAN] = {
        .decay_target = 10000,
        .sustain_target = 10000,
        .attack_step = 96,
        .decay_step = 1,
        .sustain_decay_step = 0,
        .release_step = 2,
        .sustain_decay_div = 8,
        .pluck_samples = 0,

        .h2_shift = 1, /* 1/2 */
        .h3_shift = 2, /* 1/4 */
        .h4_shift = 4, /* 1/16 */

        .pluck_h3_shift = 0,
        .pluck_h4_shift = 0,
    },

    [TIMBRE_GUITAR] = {
        .decay_target = 2600,
        .sustain_target = 500,
        .attack_step = 256,
        .decay_step = 10,
        .sustain_decay_step = 2,
        .release_step = 12,
        .sustain_decay_div = 4,
        .pluck_samples = 48,

        .h2_shift = 3, /* 1/8 */
        .h3_shift = 4, /* 1/16 */
        .h4_shift = 5, /* 1/32 */

        .pluck_h3_shift = 2, /* 1/4 */
        .pluck_h4_shift = 3, /* 1/8 */
    },
};

static int64_t scale_sample(int32_t s, uint8_t shift) {
  if (shift == 0) {
    return (int64_t)s;
  }
  return ((int64_t)s) >> shift;
}

void timbre_set_mode(TimbreMode mode) {
  if (mode < TIMBRE_PIANO || mode > TIMBRE_GUITAR) {
    return;
  }
  current_mode = mode;
}

TimbreMode timbre_get_mode(void) { return current_mode; }

const TimbreSpec* timbre_get_spec(void) { return &timbre_specs[current_mode]; }

int64_t timbre_render_raw_sample(uint32_t phase, uint8_t pluck_count,
                                 const TimbreSpec* spec) {
  if (spec == 0) {
    spec = timbre_get_spec();
  }

  uint32_t p1 = phase;
  uint32_t p2 = p1 << 1;
  uint32_t p3 = p1 * 3;
  uint32_t p4 = p1 << 2;

  int32_t s1 = osc_sample_from_phase(p1);
  int32_t s2 = osc_sample_from_phase(p2);
  int32_t s3 = osc_sample_from_phase(p3);
  int32_t s4 = osc_sample_from_phase(p4);

  int64_t raw = (int64_t)s1;
  raw += scale_sample(s2, spec->h2_shift);
  raw += scale_sample(s3, spec->h3_shift);
  raw += scale_sample(s4, spec->h4_shift);

  if (pluck_count > 0 && spec->pluck_samples > 0) {
    raw += scale_sample(s3, spec->pluck_h3_shift);
    raw += scale_sample(s4, spec->pluck_h4_shift);
  }

  return raw;
}