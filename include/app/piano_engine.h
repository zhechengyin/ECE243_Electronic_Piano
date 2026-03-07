#ifndef PIANO_ENGINE_H
#define PIANO_ENGINE_H

#include "app/key_event.h"

void piano_engine_init(void);
void piano_engine_on_key_event(KeyEvent ev);

#endif