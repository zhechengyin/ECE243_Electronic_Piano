#include "io/vga.h"

#include <stddef.h>

#include "platform/address_map.h"

/* Pixel buffer base */
#define VGA_PIXEL_BUFFER FPGA_PIXEL_BUF_BASE

/* Character buffer base */
#define VGA_CHAR_BUFFER FPGA_CHAR_BASE

/* key pressed state: 1 = highlighted, 0 = normal */
static uint8_t key_state[PIANO_KEY_COUNT] = {0};

/* ---------------------------------------------------------
 * Draw one pixel (RGB565)
 * --------------------------------------------------------- */
void vga_put_pixel(int x, int y, uint16_t color) {
  volatile uint16_t* pixel_ptr;

  if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) return;

  pixel_ptr = (volatile uint16_t*)(VGA_PIXEL_BUFFER + (y << 10) + (x << 1));

  *pixel_ptr = color;
}

/* ---------------------------------------------------------
 * Fill rectangle
 * --------------------------------------------------------- */
void vga_fill_rect(int x, int y, int width, int height, uint16_t color) {
  int row, col;

  for (row = 0; row < height; row++) {
    for (col = 0; col < width; col++) {
      vga_put_pixel(x + col, y + row, color);
    }
  }
}

/* ---------------------------------------------------------
 * Draw rectangle border
 * --------------------------------------------------------- */
void vga_draw_rect(int x, int y, int width, int height, uint16_t color) {
  int i;

  if (width <= 0 || height <= 0) return;

  for (i = 0; i < width; i++) {
    vga_put_pixel(x + i, y, color);
    vga_put_pixel(x + i, y + height - 1, color);
  }

  for (i = 0; i < height; i++) {
    vga_put_pixel(x, y + i, color);
    vga_put_pixel(x + width - 1, y + i, color);
  }
}

/* ---------------------------------------------------------
 * Clear pixel screen
 * --------------------------------------------------------- */
void vga_clear(uint16_t color) {
  int x, y;

  for (y = 0; y < SCREEN_HEIGHT; y++) {
    for (x = 0; x < SCREEN_WIDTH; x++) {
      vga_put_pixel(x, y, color);
    }
  }
}

/* ---------------------------------------------------------
 * Write one character into character buffer
 *
 * Character buffer is 80 x 60
 * --------------------------------------------------------- */
void vga_char_put(int x, int y, char c) {
  volatile char* char_ptr;

  if (x < 0 || x >= 80 || y < 0 || y >= 60) return;

  char_ptr = (volatile char*)(VGA_CHAR_BUFFER + (y << 7) + x);
  *char_ptr = c;
}

/* ---------------------------------------------------------
 * Clear character buffer
 * --------------------------------------------------------- */
void vga_char_clear(void) {
  int x, y;

  for (y = 0; y < 60; y++) {
    for (x = 0; x < 80; x++) {
      vga_char_put(x, y, ' ');
    }
  }
}

/* ---------------------------------------------------------
 * Draw labels A S D F G H J K L near bottom of white keys
 * --------------------------------------------------------- */
void vga_draw_key_labels(void) {
  static const char labels[PIANO_KEY_COUNT] = {'A', 'S', 'D', 'F', 'G',
                                               'H', 'J', 'K', 'L'};

  int i;
  int pixel_x;
  int pixel_y;
  int char_x;
  int char_y;

  /*
   * char buffer is 80x60 over 640x480
   * so:
   *   char_x = pixel_x / 8
   *   char_y = pixel_y / 8
   */

  pixel_y = PIANO_START_Y + PIANO_KEY_HEIGHT - 24;
  char_y = pixel_y / 8;

  for (i = 0; i < PIANO_KEY_COUNT; i++) {
    pixel_x = PIANO_START_X + i * PIANO_KEY_WIDTH + (PIANO_KEY_WIDTH / 2) - 4;
    char_x = pixel_x / 8;

    vga_char_put(char_x, char_y, labels[i]);
  }
}

/* ---------------------------------------------------------
 * Draw one piano key
 *
 * highlighted = true  -> red
 * highlighted = false -> white
 * --------------------------------------------------------- */
void piano_draw_key(KeyCode key, bool highlighted) {
  int key_x;
  int key_y = PIANO_START_Y;
  uint16_t fill_color;

  if (key <= KEY_NONE || key > KEY_L) return;

  key_x = PIANO_START_X + ((int)key - 1) * PIANO_KEY_WIDTH;
  fill_color = highlighted ? COLOR_RED : COLOR_WHITE;

  vga_fill_rect(key_x, key_y, PIANO_KEY_WIDTH, PIANO_KEY_HEIGHT, fill_color);

  vga_draw_rect(key_x, key_y, PIANO_KEY_WIDTH, PIANO_KEY_HEIGHT, COLOR_BLACK);
}

/* ---------------------------------------------------------
 * Draw static piano and labels
 * --------------------------------------------------------- */
void piano_draw_static(void) {
  int i;

  vga_clear(COLOR_GRAY);
  vga_char_clear();

  for (i = 0; i < PIANO_KEY_COUNT; i++) {
    key_state[i] = 0;
    piano_draw_key((KeyCode)(KEY_A + i), false);
  }

  vga_draw_key_labels();
}

/* ---------------------------------------------------------
 * Update one key from KeyEvent and redraw only that key
 * --------------------------------------------------------- */
void piano_handle_key_event(const KeyEvent* event) {
  int index;

  if (event == NULL) return;

  if (event->key < KEY_A || event->key > KEY_L) return;

  index = (int)event->key - (int)KEY_A;
  key_state[index] = event->pressed ? 1 : 0;

  piano_draw_key(event->key, key_state[index] != 0);

  /* labels may be covered visually depending on placement/style */
  vga_draw_key_labels();
}