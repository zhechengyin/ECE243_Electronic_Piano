#ifndef PIANO_ENGINE_H
#define PIANO_ENGINE_H

#include <stdint.h>
#include "app/key_event.h"

void piano_engine_init(void);
void piano_engine_on_key_event(KeyEvent ev);

int piano_engine_active_count(void);
int piano_engine_is_key_active(KeyCode key);
int32_t piano_engine_next_sample(void);

#endif