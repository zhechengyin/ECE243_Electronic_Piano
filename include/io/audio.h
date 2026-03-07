#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>

int  audio_write_space(void);                 // returns min(WSLC, WSRC)
void audio_write_sample(int32_t L, int32_t R);

#endif