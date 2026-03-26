#ifndef VGA_CORE_H
#define VGA_CORE_H

#include <stdint.h>
#include "platform/address_map.h"

#define VGA_PIXEL_BUFFER FPGA_PIXEL_BUF_BASE
#define VGA_CHAR_BUFFER  FPGA_CHAR_BASE

#define CHAR_W 80
#define CHAR_H 60

void vga_core_clear_char_buffer(void);
void vga_core_clear_screen(uint16_t color);

void vga_put_pixel(int x, int y, uint16_t color);
void vga_fill_rect(int x, int y, int width, int height, uint16_t color);
void vga_draw_rect(int x, int y, int width, int height, uint16_t color);
void vga_draw_letter(int x, int y, char c, uint16_t color);

#endif