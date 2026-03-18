#ifndef KEY_EVENT_H
#define KEY_EVENT_H

#include <stdint.h>

typedef enum {
    KEY_NONE = 0,
    KEY_A, KEY_S, KEY_D, KEY_F, KEY_G,
    KEY_H, KEY_J, KEY_K, KEY_L, KEY_W,
    KEY_E, KEY_T, KEY_Y, KEY_U, KEY_O
} KeyCode;

typedef struct {
    KeyCode key;
    uint8_t pressed;   // 1=down, 0=up
} KeyEvent;

#endif