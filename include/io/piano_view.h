#ifndef PIANO_VIEW_H
#define PIANO_VIEW_H

#include <stdbool.h>
#include "app/key_event.h"

void piano_view_reset(void);
void piano_view_draw_static(void);
void piano_view_draw_key(KeyCode key, bool highlighted);
void piano_view_flush(void);
void piano_view_handle_key_event(const KeyEvent *event);
void piano_view_draw_key_label(KeyCode key, bool highlighted);
void piano_view_draw_key_labels(void);

#endif