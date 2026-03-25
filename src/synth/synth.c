// #include "synth/synth.h"
// #include "synth/oscillator.h"

// /*
//     This program is sits between the oscillator and main program
//     It will receive the PS/2 keyboard input and use the input to control the oscillator 
//     the output will then send to the audio driver (FIFO) to generate sound
// */

// static int g_note_on = 0;
// static uint32_t g_freq_hz = 0;

// void synth_init(void) {
//     g_note_on = 0;
//     g_freq_hz = 0;
//     osc_init();
// }

// void synth_set_note(int note_on, uint32_t freq_hz) {
//     int new_on = note_on ? 1 : 0;

//     if (new_on && (!g_note_on || freq_hz != g_freq_hz)) {
//         g_freq_hz = freq_hz;
//         osc_set_frequency(freq_hz);
//     }
//     g_note_on = new_on;
// }

// int32_t synth_next_sample(void) {
//     return g_note_on ? osc_next_sample() : 0;
// }