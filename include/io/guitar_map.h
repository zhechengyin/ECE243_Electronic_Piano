#ifndef GUITAR_MAP_H
#define GUITAR_MAP_H

#include <stdbool.h>
#include <stdint.h>

#include "app/key_event.h"

typedef struct {
    bool valid;
    int string_index;   /* 0..5 means strings 6..1 */
    int fret_index;     /* 0..7 */
    uint32_t freq_hz;
} GuitarNote;

void guitar_map_init(void);
void guitar_map_select_string(KeyCode key);
bool guitar_map_is_string_key(KeyCode key);
bool guitar_map_is_fret_key(KeyCode key);
GuitarNote guitar_map_note_from_key(KeyCode key);

#endif