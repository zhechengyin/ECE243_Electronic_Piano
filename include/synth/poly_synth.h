#ifndef POLY_SYNTH_H
#define POLY_SYNTH_H

#include <stdint.h>
#include "app/key_event.h"

void poly_synth_init(void);
void poly_synth_note_on(KeyCode key, uint32_t freq_hz);
void poly_synth_note_off(KeyCode key);
int32_t poly_synth_next_sample(void);
int poly_synth_active_count(void);

#endif