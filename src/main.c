#include <stdint.h>

#include "app/key_event.h"
#include "app/piano_engine.h"
#include "io/audio.h"
#include "io/ps2.h"
#include "io/vga.h"
#include "platform/address_map.h"
#include "synth/notes.h"
#include "synth/timbre.h"

static void cycle_mode(void) {
  TimbreMode mode = timbre_get_mode();

  if (mode == TIMBRE_PIANO) {
    timbre_set_mode(TIMBRE_ORGAN);
  } else if (mode == TIMBRE_ORGAN) {
    timbre_set_mode(TIMBRE_GUITAR);
  } else {
    timbre_set_mode(TIMBRE_PIANO);
  }

  if (timbre_get_mode() == TIMBRE_GUITAR) {
    notes_set_zone(0);
  }

  vga_draw_zone_status();
}

int main(void) {
  volatile int* ps2_ptr = (int*)PS2_BASE;
  volatile int* switch_ptr = (int*)SW_BASE;
  volatile int* key_ptr = (int*)KEY_BASE;

  PS2Parser parser;
  KeyEvent ev;
  int prev_button_state = 0;

  /* -------- init -------- */
  ps2_init(&parser);
  piano_engine_init();
  piano_draw_static();
  vga_draw_zone_status();

  while (1) {
    /* ---------------------------------
     * 0) Check board pushbutton
     * KEY[0] cycles timbre mode
     * --------------------------------- */
    int key_state = *key_ptr;

    if ((key_state & 0x1) && !(prev_button_state & 0x1)) {
      cycle_mode();
    }

    prev_button_state = key_state;

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
        if (ev.pressed && timbre_get_mode() != TIMBRE_GUITAR) {
          int new_zone = -1;

          switch (ev.key) {
            case KEY_1: new_zone = 0; break;
            case KEY_2: new_zone = 1; break;
            case KEY_3: new_zone = 2; break;
            case KEY_4: new_zone = 3; break;
            case KEY_5: new_zone = 4; break;
            case KEY_6: new_zone = 5; break;
            case KEY_7: new_zone = 6; break;
            case KEY_8: new_zone = 7; break;
            default: break;
          }

          if (new_zone >= 0) {
            notes_set_zone(new_zone);
            vga_draw_zone_status();
            continue;
          }
        }

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