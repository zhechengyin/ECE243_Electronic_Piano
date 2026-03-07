#include "io/audio.h"
#include <stdint.h>
#include "platform/address_map.h"

static volatile int* const audio_ptr = (int *) AUDIO_BASE;

int audio_write_space(void) {
    int fifospace = *(audio_ptr + 1);
    int wsrc = (fifospace >> 16) & 0xFF;
    int wslc = (fifospace >> 24) & 0xFF;
    return (wslc < wsrc) ? wslc : wsrc;
}

void audio_write_sample(int32_t L, int32_t R) {
    *(audio_ptr + 2) = L;
    *(audio_ptr + 3) = R;
}