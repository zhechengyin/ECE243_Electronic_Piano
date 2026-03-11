#include "io/ps2.h"
#include <stddef.h>
#include <stdint.h>

/* =========================================================
 * Initialize the PS/2 parser state
 *
 * This function prepares the parser before processing any
 * incoming scan codes from the PS/2 keyboard.
 *
 * break_pending = 0
 * means we are NOT currently expecting a release code.
 * ========================================================= */
void ps2_init(PS2Parser* parser) {
  if (parser == NULL) {
    return;
  }

  parser->break_pending = 0;
}

/* =========================================================
 * Reset the parser state
 *
 * Clears any pending break-code state.
 * This can be used if the PS/2 stream becomes corrupted
 * or when restarting the keyboard input.
 * ========================================================= */
void ps2_reset(PS2Parser* parser) {
  if (parser == NULL) {
    return;
  }

  parser->break_pending = 0;
}

/* =========================================================
 * Convert a PS/2 scan code (Set 2) into a project key enum
 *
 * Only the required keys are supported:
 *   A S D F G H J K L
 *
 * If the scan code is not recognized, KEY_NONE is returned.
 * ========================================================= */
KeyCode ps2_scancode_to_key(uint8_t scancode) {
  switch (scancode) {
    case PS2_SCANCODE_A:
      return KEY_A;
    case PS2_SCANCODE_S:
      return KEY_S;
    case PS2_SCANCODE_D:
      return KEY_D;
    case PS2_SCANCODE_F:
      return KEY_F;
    case PS2_SCANCODE_G:
      return KEY_G;
    case PS2_SCANCODE_H:
      return KEY_H;
    case PS2_SCANCODE_J:
      return KEY_J;
    case PS2_SCANCODE_K:
      return KEY_K;
    case PS2_SCANCODE_L:
      return KEY_L;

    default:
      return KEY_NONE;
  }
}

/* =========================================================
 * Check whether a key is supported by this project
 *
 * This project only needs 9 keys (A-L).
 * Any other key is ignored.
 * ========================================================= */
bool ps2_key_supported(KeyCode key) {
  switch (key) {
    case KEY_A:
    case KEY_S:
    case KEY_D:
    case KEY_F:
    case KEY_G:
    case KEY_H:
    case KEY_J:
    case KEY_K:
    case KEY_L:
      return true;

    case KEY_NONE:
    default:
      return false;
  }
}

/* =========================================================
 * Parse one incoming PS/2 scan code byte
 *
 * This function processes a single byte from the keyboard.
 * It detects whether the byte corresponds to:
 *
 *   - Key press
 *   - Key release
 *   - Unsupported key
 *
 * PS/2 Set 2 release format:
 *
 *   F0 + <scan code>
 *
 *   true  -> a complete KeyEvent was generated
 *   false -> no event yet (e.g., waiting for release code)
 * ========================================================= */
bool ps2_parse_byte(PS2Parser* parser, uint8_t byte, KeyEvent* event) {
  KeyCode key;

  if (parser == NULL || event == NULL) {
    return false;
  }

  if (byte == PS2_BREAK_CODE) {
    parser->break_pending = 1;
    return false;
  }

  key = ps2_scancode_to_key(byte);

  if (!ps2_key_supported(key)) {
    parser->break_pending = 0;
    return false;
  }

  event->key = key;
  event->pressed = (parser->break_pending == 0) ? 1u : 0u;

  parser->break_pending = 0;
  return true;
}

/* =========================================================
 * Convert key enum to readable string
 *
 * Useful for debugging output:
 *
 * printf("Key %s pressed=%d\n", ps2_key_to_string(ev.key), ev.pressed);
 * ========================================================= */
const char* ps2_key_to_string(KeyCode key) {
  switch (key) {
    case KEY_A:
      return "A";
    case KEY_S:
      return "S";
    case KEY_D:
      return "D";
    case KEY_F:
      return "F";
    case KEY_G:
      return "G";
    case KEY_H:
      return "H";
    case KEY_J:
      return "J";
    case KEY_K:
      return "K";
    case KEY_L:
      return "L";

    case KEY_NONE:
      return "NONE";

    default:
      return "UNKNOWN";
  }
}

// int main() {
//   PS2Parser parser;
//   KeyEvent event;

//   ps2_init(&parser);

//   printf("=== PS2 Parser Test ===\n\n");

//   /* --------------------------------------
//      Test case 1 : A key press
//      scan code = 1C
//   -------------------------------------- */

//   printf("Input: 0x1C\n");

//   if (ps2_parse_byte(&parser, 0x1C, &event)) {
//     printf("Event -> key=%s pressed=%d\n", ps2_key_to_string(event.key),
//            event.pressed);
//   }

//   printf("\n");

//   /* --------------------------------------
//      Test case 2 : A key release
//      scan code = F0 1C
//   -------------------------------------- */

//   printf("Input: 0xF0\n");
//   ps2_parse_byte(&parser, 0xF0, &event);

//   printf("Input: 0x1C\n");

//   if (ps2_parse_byte(&parser, 0x1C, &event)) {
//     printf("Event -> key=%s pressed=%d\n", ps2_key_to_string(event.key),
//            event.pressed);
//   }

//   printf("\n");

//   /* --------------------------------------
//      Test case 3 : S key press
//   -------------------------------------- */

//   printf("Input: 0x1B\n");

//   if (ps2_parse_byte(&parser, 0x1B, &event)) {
//     printf("Event -> key=%s pressed=%d\n", ps2_key_to_string(event.key),
//            event.pressed);
//   }

//   printf("\n");

//   /* --------------------------------------
//      Test case 4 : S key release
//   -------------------------------------- */

//   printf("Input: 0xF0\n");
//   ps2_parse_byte(&parser, 0xF0, &event);

//   printf("Input: 0x1B\n");

//   if (ps2_parse_byte(&parser, 0x1B, &event)) {
//     printf("Event -> key=%s pressed=%d\n", ps2_key_to_string(event.key),
//            event.pressed);
//   }

//   printf("\n");

//   /* --------------------------------------
//      Test case 5 : unsupported key
//      (example: 0x15)
//   -------------------------------------- */

//   printf("Input: unsupported scan code 0x15\n");

//   if (!ps2_parse_byte(&parser, 0x15, &event)) {
//     printf("Ignored unsupported key\n");
//   }

//   printf("\n");

//   printf("=== Test Finished ===\n");

//   return 0;
// }