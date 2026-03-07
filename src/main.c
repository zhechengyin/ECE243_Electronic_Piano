#include <stdint.h>
#include "io/audio.h"
#include "synth/oscillator.h"
#include "platform/address_map.h"

int main (void) {
    // Temperary use switch to mute the sound
    volatile int* switch_ptr = (int *) SW_BASE;

    osc_init();   // initialize the oscillator
    osc_set_frequency(440); // set frequency to A4 (440 Hz)

    while (1){
        int note_on = (*switch_ptr) & 0x1; // check if SW0 is on

        int space = audio_write_space();   // check how many samples we can write
        if (space > 32) space = 32; // limit to 32 samples at a time
        // write 'space' samples to the audio output
        for (int i = 0; i < space; i++){
            int16_t sample = note_on ?  osc_next_sample() : 0; // get next sample from oscillator or silence
            audio_write_sample((int32_t)sample, (int32_t)sample); // write same sample to both L and R channels
        }
    }
}