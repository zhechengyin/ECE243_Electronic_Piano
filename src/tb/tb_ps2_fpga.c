#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "io/ps2.h"
#include "platform/address_map.h"

#define PS2_DATA_REG (*(volatile uint32_t*)(PS2_BASE))

#define PS2_RVALID_MASK 0x8000
#define PS2_BYTE_MASK 0x00FF

static bool ps2_hw_read_byte(uint8_t* byte_out) {
  uint32_t data;

  if (byte_out == NULL) return false;

  data = *(volatile uint32_t*)PS2_BASE;

  if (!(data & PS2_RVALID_MASK)) return false;

  *byte_out = data & PS2_BYTE_MASK;

  return true;
}

/* ---------------------------------------------------------
 * Print one parsed key event
 * --------------------------------------------------------- */
static void print_key_event(uint8_t raw_byte, const KeyEvent* event) {
  if (event == NULL) {
    return;
  }

  printf("raw=0x%02X -> key=%s pressed=%u\r\n", raw_byte,
         ps2_key_to_string(event->key), event->pressed);
}

/* ---------------------------------------------------------
 * Main board-level PS/2 test
 * --------------------------------------------------------- */
int main(void) {
  PS2Parser parser;
  KeyEvent event;
  uint8_t byte;

  ps2_init(&parser);

  printf("=====================================\r\n");
  printf(" DE1-SoC PS/2 Parser Board Test Start\r\n");
  printf(" Press A S D F G H J K L on keyboard \r\n");
  printf(" pressed=1 means key down            \r\n");
  printf(" pressed=0 means key up              \r\n");
  printf("=====================================\r\n");

  while (1) {
    if (ps2_hw_read_byte(&byte)) {
      printf("raw = 0x%02X\r\n", byte);

      if (ps2_parse_byte(&parser, byte, &event)) {
        print_key_event(byte, &event);
      }
    }
  }
  return 0;
}