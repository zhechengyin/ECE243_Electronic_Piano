#ifndef TIMBRE_H
#define TIMBRE_H

#include <stdint.h>

typedef enum {
  TIMBRE_PIANO = 0,
  TIMBRE_ORGAN = 1,
  TIMBRE_GUITAR = 2
} TimbreMode;

typedef struct {
  /* envelope behavior */
  uint16_t decay_target;
  uint16_t sustain_target;

  uint16_t attack_step;
  uint16_t decay_step;
  uint16_t sustain_decay_step;
  uint16_t release_step;

  uint8_t sustain_decay_div;
  uint8_t pluck_samples;

  /* harmonic recipe:
   * contribution = sample >> shift
   * e.g. shift 2 = 1/4, shift 3 = 1/8
   */
  uint8_t h2_shift;
  uint8_t h3_shift;
  uint8_t h4_shift;

  /* extra short attack transient */
  uint8_t pluck_h3_shift;
  uint8_t pluck_h4_shift;
} TimbreSpec;

void timbre_set_mode(TimbreMode mode);
TimbreMode timbre_get_mode(void);
const TimbreSpec* timbre_get_spec(void);

/* Render one raw voice sample from phase + timbre spec.
 * This does NOT apply envelope and does NOT mix voices.
 */
int64_t timbre_render_raw_sample(uint32_t phase, uint8_t pluck_count,
                                 const TimbreSpec* spec);

#endif