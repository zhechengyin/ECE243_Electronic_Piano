#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "io/ps2.h"
#include "io/vga.h"
#include "platform/address_map.h"

#define PS2_DATA_REG (*(volatile uint32_t*)(PS2_BASE))
#define PS2_RVALID_MASK 0x8000
#define PS2_BYTE_MASK 0x00FF

static bool ps2_hw_read_byte(uint8_t* byte_out) {
  uint32_t data;

  if (byte_out == NULL) return false;

  data = PS2_DATA_REG;

  if (!(data & PS2_RVALID_MASK)) return false;

  *byte_out = (uint8_t)(data & PS2_BYTE_MASK);
  return true;
}

int main(void) {
  PS2Parser parser;
  KeyEvent event;
  uint8_t byte;

  ps2_init(&parser);
  piano_draw_static();

  printf("PS2 + VGA piano test start\r\n");

  while (1) {
    if (ps2_hw_read_byte(&byte)) {
      printf("raw = 0x%02X\r\n", byte);

      if (ps2_parse_byte(&parser, byte, &event)) {
        printf("key=%s pressed=%u\r\n", ps2_key_to_string(event.key),
               event.pressed);

        piano_handle_key_event(&event);
      }
    }
  }

  return 0;
}