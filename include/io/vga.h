#ifndef VGA_H
#define VGA_H

#include <stdbool.h>
#include <stdint.h>

#include "app/key_event.h"

/* Screen layout */
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

/* Piano layout */
#define PIANO_KEY_COUNT 9
#define PIANO_KEY_WIDTH 60
#define PIANO_KEY_HEIGHT 200
#define PIANO_START_X 50
#define PIANO_START_Y 140

/* Colors (RGB565) */
#define COLOR_BLACK 0x0000
#define COLOR_WHITE 0xFFFF
#define COLOR_RED 0xF800
#define COLOR_GRAY 0x8410

void vga_put_pixel(int x, int y, uint16_t color);
void vga_fill_rect(int x, int y, int width, int height, uint16_t color);
void vga_draw_rect(int x, int y, int width, int height, uint16_t color);
void vga_clear(uint16_t color);

void vga_char_put(int x, int y, char c);
void vga_char_clear(void);
void vga_draw_key_labels(void);

void piano_draw_static(void);
void piano_draw_key(KeyCode key, bool highlighted);
void piano_handle_key_event(const KeyEvent* event);

#endif