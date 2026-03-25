#ifndef NOTES_H
#define NOTES_H

#include <stdint.h>
#include "app/key_event.h"

// Return frequency in Hz for a given KeyCode.
// If key is not mapped, return 0.
uint32_t note_freq_from_keycode(KeyCode k);
void notes_set_zone(int z);
int  notes_get_zone(void);
int  notes_get_zone_count(void);

#endif