#include "io/vga_core.h"

#include <stdint.h>
#include "platform/address_map.h"
#include "io/vga.h"

void vga_core_clear_char_buffer(void) {
    volatile char *char_buf = (volatile char *)VGA_CHAR_BUFFER;

    for (int y = 0; y < CHAR_H; y++) {
        for (int x = 0; x < CHAR_W; x++) {
            char_buf[(y << 7) + x] = ' ';
        }
    }
}

void vga_put_pixel(int x, int y, uint16_t color) {
    if (x < 0 || x >= SCREEN_W || y < 0 || y >= SCREEN_H) {
        return;
    }

    volatile uint16_t *pixel =
        (volatile uint16_t *)(VGA_PIXEL_BUFFER + (y << 10) + (x << 1));
    *pixel = color;
}

void vga_fill_rect(int x, int y, int width, int height, uint16_t color) {
    int x0 = x;
    int y0 = y;
    int x1 = x + width;
    int y1 = y + height;

    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 > SCREEN_W) x1 = SCREEN_W;
    if (y1 > SCREEN_H) y1 = SCREEN_H;

    for (int yy = y0; yy < y1; yy++) {
        for (int xx = x0; xx < x1; xx++) {
            vga_put_pixel(xx, yy, color);
        }
    }
}

void vga_draw_rect(int x, int y, int width, int height, uint16_t color) {
    if (width <= 0 || height <= 0) {
        return;
    }

    for (int xx = x; xx < x + width; xx++) {
        vga_put_pixel(xx, y, color);
        vga_put_pixel(xx, y + height - 1, color);
    }

    for (int yy = y; yy < y + height; yy++) {
        vga_put_pixel(x, yy, color);
        vga_put_pixel(x + width - 1, yy, color);
    }
}

void vga_core_clear_screen(uint16_t color) {
    vga_fill_rect(0, 0, SCREEN_W, SCREEN_H, color);
}

/* 5x7 bitmap font, scaled x2 */
static void draw_key_letter_pixels(int x, int y, char c, uint16_t color) {
    static const uint8_t FONT_A[7] = {0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11};
    static const uint8_t FONT_D[7] = {0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E};
    static const uint8_t FONT_E[7] = {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F};
    static const uint8_t FONT_F[7] = {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10};
    static const uint8_t FONT_G[7] = {0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0E};
    static const uint8_t FONT_H[7] = {0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11};
    static const uint8_t FONT_J[7] = {0x07, 0x02, 0x02, 0x02, 0x12, 0x12, 0x0C};
    static const uint8_t FONT_M[7] = {0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11};
    static const uint8_t FONT_N[7] = {0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11};
    static const uint8_t FONT_O[7] = {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E};
    static const uint8_t FONT_P[7] = {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10};
    static const uint8_t FONT_R[7] = {0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11};
    static const uint8_t FONT_S[7] = {0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E};
    static const uint8_t FONT_T[7] = {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04};
    static const uint8_t FONT_U[7] = {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E};
    static const uint8_t FONT_W[7] = {0x11, 0x11, 0x11, 0x15, 0x15, 0x15, 0x0A};
    static const uint8_t FONT_Y[7] = {0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04};

    static const uint8_t FONT_1[7] = {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E};
    static const uint8_t FONT_2[7] = {0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F};
    static const uint8_t FONT_3[7] = {0x1E, 0x01, 0x01, 0x0E, 0x01, 0x01, 0x1E};
    static const uint8_t FONT_4[7] = {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02};
    static const uint8_t FONT_5[7] = {0x1F, 0x10, 0x10, 0x1E, 0x01, 0x01, 0x1E};
    static const uint8_t FONT_6[7] = {0x0E, 0x10, 0x10, 0x1E, 0x11, 0x11, 0x0E};
    static const uint8_t FONT_7[7] = {0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08};
    static const uint8_t FONT_8[7] = {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E};

    const uint8_t *glyph = 0;
    const int scale = 2;

    switch (c) {
        case 'A': glyph = FONT_A; break;
        case 'D': glyph = FONT_D; break;
        case 'E': glyph = FONT_E; break;
        case 'F': glyph = FONT_F; break;
        case 'G': glyph = FONT_G; break;
        case 'H': glyph = FONT_H; break;
        case 'J': glyph = FONT_J; break;
        case 'M': glyph = FONT_M; break;
        case 'N': glyph = FONT_N; break;
        case 'O': glyph = FONT_O; break;
        case 'P': glyph = FONT_P; break;
        case 'R': glyph = FONT_R; break;
        case 'S': glyph = FONT_S; break;
        case 'T': glyph = FONT_T; break;
        case 'U': glyph = FONT_U; break;
        case 'W': glyph = FONT_W; break;
        case 'Y': glyph = FONT_Y; break;
        case '1': glyph = FONT_1; break;
        case '2': glyph = FONT_2; break;
        case '3': glyph = FONT_3; break;
        case '4': glyph = FONT_4; break;
        case '5': glyph = FONT_5; break;
        case '6': glyph = FONT_6; break;
        case '7': glyph = FONT_7; break;
        case '8': glyph = FONT_8; break;
        default: return;
    }

    for (int row = 0; row < 7; row++) {
        for (int col = 0; col < 5; col++) {
            if (glyph[row] & (1 << (4 - col))) {
                vga_fill_rect(x + col * scale, y + row * scale, scale, scale, color);
            }
        }
    }
}

void vga_draw_letter(int x, int y, char c, uint16_t color) {
    draw_key_letter_pixels(x, y, c, color);
}