#ifndef OSCILLATOR_H
#define OSCILLATOR_H

#include <stdint.h>

void osc_init(void);
void osc_set_frequency(uint32_t freq_hz);
int32_t osc_next_sample(void);

uint32_t osc_phase_step_from_freq(uint32_t freq_hz);
int32_t osc_sample_from_phase(uint32_t phase);

#endif