#include <stdint.h>

#include "app/key_event.h"
#include "app/piano_engine.h"
#include "io/audio.h"
#include "io/ps2.h"
#include "io/vga.h"
#include "platform/address_map.h"

int main(void) {
  volatile int* ps2_ptr = (int*)PS2_BASE;
  volatile int* switch_ptr = (int*)SW_BASE;

  PS2Parser parser;
  KeyEvent ev;

  /* -------- init -------- */
  ps2_init(&parser);
  piano_engine_init();
  piano_draw_static();

  while (1) {
    /* ---------------------------------
     * 1) Drain PS/2 FIFO first
     * --------------------------------- */
    while (1) {
      int ps2_data = *ps2_ptr;
      int rvalid = (ps2_data >> 15) & 0x1;

      if (!rvalid) {
        break;
      }

      uint8_t byte = (uint8_t)(ps2_data & 0xFF);

      if (ps2_parse_byte(&parser, byte, &ev)) {
        /* audio side */
        piano_engine_on_key_event(ev);

        /* VGA side */
        piano_handle_key_event(&ev);
      }
    }

    piano_vga_flush();

    /* ---------------------------------
     * 2) Feed audio FIFO
     * --------------------------------- */
    int space = audio_write_space();
    if (space > 32) {
      space = 32;
    }

    int sound_enable = (*switch_ptr) & 0x1;

    for (int i = 0; i < space; i++) {
      int32_t sample = sound_enable ? piano_engine_next_sample() : 0;
      audio_write_sample(sample, sample);
    }
  }
}