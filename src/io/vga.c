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
 * Draw a filled block for a font pixel
 * scale = 2 makes letters easier to see
 * --------------------------------------------------------- */
static void vga_draw_font_block(int x, int y, uint16_t color)
{
    int dx, dy;
    const int scale = 2;

    for (dy = 0; dy < scale; dy++)
    {
        for (dx = 0; dx < scale; dx++)
        {
            vga_put_pixel(x + dx, y + dy, color);
        }
    }
}

/* ---------------------------------------------------------
 * Draw one 5x7 style letter using pixel buffer
 * Only supports: A S D F G H J K L
 * --------------------------------------------------------- */
void vga_draw_letter(int x, int y, char c, uint16_t color)
{
    int row, col;

    static const uint8_t font_A[7] = {
        0x0E, /* 01110 */
        0x11, /* 10001 */
        0x11, /* 10001 */
        0x1F, /* 11111 */
        0x11, /* 10001 */
        0x11, /* 10001 */
        0x11  /* 10001 */
    };

    static const uint8_t font_S[7] = {
        0x0F, /* 01111 */
        0x10, /* 10000 */
        0x10, /* 10000 */
        0x0E, /* 01110 */
        0x01, /* 00001 */
        0x01, /* 00001 */
        0x1E  /* 11110 */
    };

    static const uint8_t font_D[7] = {
        0x1E, /* 11110 */
        0x11, /* 10001 */
        0x11, /* 10001 */
        0x11, /* 10001 */
        0x11, /* 10001 */
        0x11, /* 10001 */
        0x1E  /* 11110 */
    };

    static const uint8_t font_F[7] = {
        0x1F, /* 11111 */
        0x10, /* 10000 */
        0x10, /* 10000 */
        0x1E, /* 11110 */
        0x10, /* 10000 */
        0x10, /* 10000 */
        0x10  /* 10000 */
    };

    static const uint8_t font_G[7] = {
        0x0E, /* 01110 */
        0x11, /* 10001 */
        0x10, /* 10000 */
        0x17, /* 10111 */
        0x11, /* 10001 */
        0x11, /* 10001 */
        0x0E  /* 01110 */
    };

    static const uint8_t font_H[7] = {
        0x11, /* 10001 */
        0x11, /* 10001 */
        0x11, /* 10001 */
        0x1F, /* 11111 */
        0x11, /* 10001 */
        0x11, /* 10001 */
        0x11  /* 10001 */
    };

    static const uint8_t font_J[7] = {
        0x07, /* 00111 */
        0x02, /* 00010 */
        0x02, /* 00010 */
        0x02, /* 00010 */
        0x12, /* 10010 */
        0x12, /* 10010 */
        0x0C  /* 01100 */
    };

    static const uint8_t font_K[7] = {
        0x11, /* 10001 */
        0x12, /* 10010 */
        0x14, /* 10100 */
        0x18, /* 11000 */
        0x14, /* 10100 */
        0x12, /* 10010 */
        0x11  /* 10001 */
    };

    static const uint8_t font_L[7] = {
        0x10, /* 10000 */
        0x10, /* 10000 */
        0x10, /* 10000 */
        0x10, /* 10000 */
        0x10, /* 10000 */
        0x10, /* 10000 */
        0x1F  /* 11111 */
    };

    const uint8_t *glyph = NULL;

    switch (c)
    {
        case 'A': glyph = font_A; break;
        case 'S': glyph = font_S; break;
        case 'D': glyph = font_D; break;
        case 'F': glyph = font_F; break;
        case 'G': glyph = font_G; break;
        case 'H': glyph = font_H; break;
        case 'J': glyph = font_J; break;
        case 'K': glyph = font_K; break;
        case 'L': glyph = font_L; break;
        default: return;
    }

    for (row = 0; row < 7; row++)
    {
        for (col = 0; col < 5; col++)
        {
            if (glyph[row] & (1 << (4 - col)))
            {
                vga_draw_font_block(x + col * 2, y + row * 2, color);
            }
        }
    }
}

/* ---------------------------------------------------------
 * Clear character buffer (80x60)
 * --------------------------------------------------------- */
void vga_char_clear(void)
{
    volatile char *char_ptr;
    int x, y;

    for (y = 0; y < 60; y++)
    {
        for (x = 0; x < 80; x++)
        {
            char_ptr = (volatile char *)(VGA_CHAR_BUFFER + (y << 7) + x);
            *char_ptr = ' ';
        }
    }
}

/* ---------------------------------------------------------
 * Draw one key label directly on the pixel buffer
 * White key  -> black letter
 * Red key    -> white letter
 * --------------------------------------------------------- */
void vga_draw_key_label(KeyCode key, bool highlighted)
{
    int key_x;
    int key_y;
    int letter_x;
    int letter_y;
    uint16_t letter_color;
    char letter = '?';

    if (key < KEY_A || key > KEY_L)
        return;

    key_x = PIANO_START_X + ((int)key - 1) * PIANO_KEY_WIDTH;
    key_y = PIANO_START_Y;

    switch (key)
    {
        case KEY_A: letter = 'A'; break;
        case KEY_S: letter = 'S'; break;
        case KEY_D: letter = 'D'; break;
        case KEY_F: letter = 'F'; break;
        case KEY_G: letter = 'G'; break;
        case KEY_H: letter = 'H'; break;
        case KEY_J: letter = 'J'; break;
        case KEY_K: letter = 'K'; break;
        case KEY_L: letter = 'L'; break;
        default: return;
    }

    letter_color = highlighted ? COLOR_WHITE : COLOR_BLACK;

    /* roughly centered near lower-middle of the key */
    letter_x = key_x + (PIANO_KEY_WIDTH / 2) - 6;
    letter_y = key_y + PIANO_KEY_HEIGHT - 32;

    vga_draw_letter(letter_x, letter_y, letter, letter_color);
}

/* ---------------------------------------------------------
 * Draw one piano key
 *
 * highlighted = true  -> red
 * highlighted = false -> white
 * --------------------------------------------------------- */
void piano_draw_key(KeyCode key, bool highlighted)
{
    int key_x;
    int key_y = PIANO_START_Y;
    uint16_t fill_color;

    if (key <= KEY_NONE || key > KEY_L)
        return;

    key_x = PIANO_START_X + ((int)key - 1) * PIANO_KEY_WIDTH;
    fill_color = highlighted ? COLOR_RED : COLOR_WHITE;

    vga_fill_rect(key_x, key_y, PIANO_KEY_WIDTH, PIANO_KEY_HEIGHT, fill_color);

    vga_draw_rect(key_x, key_y, PIANO_KEY_WIDTH, PIANO_KEY_HEIGHT, COLOR_BLACK);

    /* always redraw label with contrasting color */
    vga_draw_key_label(key, highlighted);
}

/* ---------------------------------------------------------
 * Draw static piano and labels
 * --------------------------------------------------------- */
void piano_draw_static(void)
{
    int i;

    vga_clear(COLOR_GRAY);
    vga_char_clear();   // clear old small letters

    for (i = 0; i < PIANO_KEY_COUNT; i++)
    {
        key_state[i] = 0;
        piano_draw_key((KeyCode)(KEY_A + i), false);
    }
}

/* ---------------------------------------------------------
 * Update one key from KeyEvent and redraw only that key
 * --------------------------------------------------------- */
void piano_handle_key_event(const KeyEvent *event)
{
    int index;

    if (event == NULL)
        return;

    if (event->key < KEY_A || event->key > KEY_L)
        return;

    index = (int)event->key - (int)KEY_A;
    key_state[index] = event->pressed ? 1 : 0;

    piano_draw_key(event->key, key_state[index] != 0);
}