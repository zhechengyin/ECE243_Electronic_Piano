#include "app/piano_engine.h"
#include "synth/synth.h"
#include "synth/notes.h"

/*
    This module is the "brain" of the piano application.
    It receives key events from the PS/2 input, maps them to musical notes,
    and controls the synthesizer accordingly.
*/

static int current_on = 0;
static uint32_t current_freq = 440;

void piano_engine_init(void) {
    synth_init();
    current_on = 0;
    current_freq = 440;
}

void piano_engine_on_key_event(KeyEvent ev) {
    uint32_t f = note_freq_from_keycode(ev.key);
    
    if (f == 0) return; // ignore unmapped keys

    if (ev.pressed) {
        current_on = 1;
        current_freq = f;
        synth_set_note(current_on, current_freq);
    } else {
        current_on = 0;
        synth_set_note(current_on, current_freq);
    }
}