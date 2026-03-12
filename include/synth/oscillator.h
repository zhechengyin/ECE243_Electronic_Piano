#ifndef OSCILLATOR_H
#define OSCILLATOR_H

#include <stdint.h>

void osc_init(void);
void osc_set_frequency(uint32_t freq_hz);
int32_t osc_next_sample(void);

#endif