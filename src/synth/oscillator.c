#include "synth/oscillator.h"
#include <stdint.h>
#include <math.h>

/*
    This is a simple implementation of a sine wave oscillator using a lookup table.
    The oscillator generates audio samples at a fixed sample rate (e.g., 8000 Hz).
    The frequency can be set by calculating the appropriate step size for the phase accumulator.
*/

#define SAMPLE_RATE 8000
#define TABLE_SIZE 256
#define AMPLITUDE 0x00300000

static int32_t sine_table[TABLE_SIZE];
static uint32_t phase = 0;                  // An indicator of the current position
static uint32_t step = 0;                   // Step size for each sample

void osc_init(void) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        double x = 2.0 * M_PI * (double)i / (double)TABLE_SIZE;
        sine_table[i] = (int32_t)(AMPLITUDE * sin(x));
    }
    phase = 0;
    step = 0;
}

uint32_t osc_phase_step_from_freq(uint32_t freq_hz) {
    return ((uint64_t)freq_hz << 32) / SAMPLE_RATE;
}

int32_t osc_sample_from_phase(uint32_t phase_value) {
    uint8_t idx = phase_value >> 24;
    return sine_table[idx];
}

// Not gonna use but legal for singal voice testing
void osc_set_frequency(uint32_t freq_hz) {
    step = osc_phase_step_from_freq(freq_hz);
}

int32_t osc_next_sample(void) {
    phase += step;
    return osc_sample_from_phase(phase);
}