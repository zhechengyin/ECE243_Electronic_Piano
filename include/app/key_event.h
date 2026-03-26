#ifndef KEY_EVENT_H
#define KEY_EVENT_H

#include <stdint.h>

typedef enum {
    KEY_NONE = 0,

    KEY_A, KEY_W, KEY_S, KEY_E, KEY_D,
    KEY_F, KEY_T, KEY_G, KEY_Y, KEY_H,
    KEY_U, KEY_J, 
    KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, 
    KEY_6, KEY_7, KEY_8,

    KEY_COUNT
} KeyCode;

typedef struct {
    KeyCode key;
    uint8_t pressed;   // 1=down, 0=up
} KeyEvent;

#endif