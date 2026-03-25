/* io/vga.h */
/* io/vga.h */
#ifndef VGA_H
#define VGA_H

#include <stdbool.h>
#include <stdint.h>
#include "app/key_event.h"

#define WHITE_W      30
#define WHITE_H      140
#define BLACK_W      18
#define BLACK_H      85
#define SCREEN_H      240
#define SCREEN_W     320

#define NUM_WHITE    9
#define NUM_BLACK    6

#define KB_X         ((SCREEN_W - (NUM_WHITE * WHITE_W)) / 2)
#define KB_Y         (SCREEN_H - WHITE_H - 8)

#define COLOR_BG         0xC618
#define COLOR_WHITE      0xFFFF
#define COLOR_WHITE_ON   0xFFE0
#define COLOR_BLACK      0x0000
#define COLOR_BLACK_ON   0xF800
#define COLOR_BORDER     0x0000
#define WHITE_W      30
#define WHITE_H      140
#define BLACK_W      18
#define BLACK_H      85
#define SCREEN_H      240
#define SCREEN_W     320

#define NUM_WHITE    9
#define NUM_BLACK    6

#define KB_X         ((SCREEN_W - (NUM_WHITE * WHITE_W)) / 2)
#define KB_Y         (SCREEN_H - WHITE_H - 8)

#define COLOR_BG         0xC618
#define COLOR_WHITE      0xFFFF
#define COLOR_WHITE_ON   0xFFE0
#define COLOR_BLACK      0x0000
#define COLOR_BLACK_ON   0xF800
#define COLOR_BORDER     0x0000

void vga_put_pixel(int x, int y, uint16_t color);
void vga_fill_rect(int x, int y, int width, int height, uint16_t color);
void vga_draw_rect(int x, int y, int width, int height, uint16_t color);
void vga_clear(uint16_t color);

void vga_draw_letter(int x, int y, char c, uint16_t color);
void vga_draw_letter(int x, int y, char c, uint16_t color);
void vga_draw_key_labels(void);
void vga_draw_key_label(KeyCode key, bool highlighted);
void vga_draw_key_label(KeyCode key, bool highlighted);

void piano_draw_static(void);
void piano_draw_key(KeyCode key, bool highlighted);
void piano_handle_key_event(const KeyEvent *event);
void piano_vga_flush(void);
void vga_draw_zone_status(void);

#endif