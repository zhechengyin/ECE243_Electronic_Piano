#ifndef GUITAR_VIEW_H
#define GUITAR_VIEW_H

#include <stdbool.h>
#include "app/key_event.h"

void guitar_view_draw_static(void);
void guitar_view_handle_key_event(const KeyEvent *event);

#endif