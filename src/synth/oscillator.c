#include "synth/oscillator.h"
#include <stdint.h>
#include <math.h>

#define SAMPLE_RATE 8000
#define TABLE_SIZE 256
# define AMPLITUDE 12000

static int16_t sine_table[TABLE_SIZE];
static uint32_t phase = 0;                  // An indicator of the current position
static uint32_t step = 0;                   // Step size for each sample

void osc_init(void) {
    // fill sine_table once using sin()
    // reset phase
    for (int i = 0; i < TABLE_SIZE; i++) {
        double x = 2.0 * M_PI * (double)i / (double)TABLE_SIZE;
        sine_table[i] = (int16_t)(AMPLITUDE * sin(x));
    }
    phase = 0;
    step = 0;
}

void osc_set_frequency(uint32_t freq_hz) {
    // step = freq * 2^32 / SAMPLE_RATE
    step = ((uint64_t)freq_hz << 32) / SAMPLE_RATE;
}

int16_t osc_next_sample(void) {
    // phase += step
    // index = phase >> 24
    // return sine_table[index]
    phase += step;
    uint8_t idx = phase >> 24; // get the top 8 bits for index (0-255)
    return sine_table[idx];
}