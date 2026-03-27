#ifndef APP_PIANO_ENGINE_H
#define APP_PIANO_ENGINE_H

#include <stdint.h>

#include "app/key_event.h"

void piano_engine_init(void);
void piano_engine_on_key_event(KeyEvent ev);
int32_t piano_engine_next_sample(void);
void piano_engine_all_notes_off(void);

#endif
