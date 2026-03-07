#ifndef NOTE_EVENT_H
#define NOTE_EVENT_H

#include <stdint.h>

typedef struct {
    uint8_t pressed;     // 1 down, 0 up
    uint32_t freq_hz;    // already mapped to frequency
} NoteEvent;

#endif