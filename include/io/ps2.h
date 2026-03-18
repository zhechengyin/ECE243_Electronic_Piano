#ifndef PS2_H
#define PS2_H

#include <stdbool.h>
#include <stdint.h>
#include "app/key_event.h"

/* =========================================================
 *  PS/2 Scan Code Set 2 constants
 * ========================================================= */

#define PS2_BREAK_CODE     0xF0

#define PS2_SCANCODE_A     0x1C
#define PS2_SCANCODE_S     0x1B
#define PS2_SCANCODE_D     0x23
#define PS2_SCANCODE_F     0x2B
#define PS2_SCANCODE_G     0x34
#define PS2_SCANCODE_H     0x33
#define PS2_SCANCODE_J     0x3B
#define PS2_SCANCODE_K     0x42
#define PS2_SCANCODE_L     0x4B
#define PS2_SCANCODE_W     0x1D
#define PS2_SCANCODE_E     0x24
#define PS2_SCANCODE_T     0x2C
#define PS2_SCANCODE_Y     0x35
#define PS2_SCANCODE_U     0x3C
#define PS2_SCANCODE_O     0x44

/* =========================================================
 *  PS/2 Parser state
 *
 *  break_pending = 1 means previous byte was F0
 * ========================================================= */

typedef struct {
    uint8_t break_pending;   // 1 if waiting for release code, 0 otherwise
} PS2Parser;


/* =========================================================
 *  Parser control
 * ========================================================= */

/* Initialize parser state */
void ps2_init(PS2Parser *parser);

/* Reset parser state */
void ps2_reset(PS2Parser *parser);

/* =========================================================
 *  Scan code processing
 * ========================================================= */

/* Convert scan code to KeyCode */
KeyCode ps2_scancode_to_key(uint8_t scancode);

/*
 * Parse one incoming scan code byte
 *
 * Return:
 *   true  -> a complete KeyEvent is generated
 *   false -> no event yet
 */
bool ps2_parse_byte(PS2Parser *parser, uint8_t byte, KeyEvent *event);


/* =========================================================
 *  Utility functions
 * ========================================================= */

/* Check if key is supported */
bool ps2_key_supported(KeyCode key);

/* Convert key to readable string (debugging) */
const char *ps2_key_to_string(KeyCode key);


#endif // PS2_H