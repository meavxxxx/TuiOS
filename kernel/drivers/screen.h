#ifndef SCREEN_H
#define SCREEN_H

#include "../libc/stdint.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

typedef enum {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
} vga_color_t;

void screen_init(void);
void screen_clear(void);
void screen_putchar(char c);
void screen_write(const char* str);
void screen_setcolor(uint8_t fg, uint8_t bg);
void screen_scroll(void);

void kprint(const char* str);
void kprint_hex(uint32_t n);
void kprint_dec(uint32_t n);

#endif