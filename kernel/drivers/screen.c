#include "screen.h"
#include "../cpu/ports.h"

static volatile uint16_t* vga_buffer = (uint16_t*)0xB8000;

static uint8_t cursor_x = 0;
static uint8_t cursor_y = 0;

static uint8_t current_color = 0x0F;

static inline uint16_t vga_entry(char c, uint8_t color) {
    return (uint16_t)c | (uint16_t)color << 8;
}

static inline uint8_t vga_color(vga_color_t fg, vga_color_t bg) {
    return fg | bg << 4;
}

static void update_cursor(void) {
    uint16_t pos = cursor_y * VGA_WIDTH + cursor_x;

    port_byte_out(0x3D4, 0x0F);
    port_byte_out(0x3D5, (uint8_t)(pos & 0xFF));
    port_byte_out(0x3D4, 0x0E);
    port_byte_out(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void screen_init(void) {
    cursor_x = 0;
    cursor_y = 0;
    current_color = vga_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
}

void screen_clear(void) {
    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            const int index = y * VGA_WIDTH + x;
            vga_buffer[index] = vga_entry(' ', current_color);
        }
    }
    cursor_x = 0;
    cursor_y = 0;
    update_cursor();
}
void screen_scroll(void) {
    for (int y = 0; y < VGA_HEIGHT - 1; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[y * VGA_WIDTH + x] = vga_buffer[(y + 1) * VGA_WIDTH + x]; 
        }
    }

    for (int x = 0; x < VGA_WIDTH; x++) {
        vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', current_color);
    }

    cursor_y = VGA_HEIGHT - 1;
}

void screen_putchar(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\r') {
        cursor_x = 0;
    } else if (c == '\t') {
        cursor_x = (cursor_x + 4) & ~(4 - 1);
    } else if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
            vga_buffer[cursor_y * VGA_WIDTH + cursor_x] = vga_entry(' ', current_color);
        }
    } else {
        vga_buffer[cursor_y * VGA_WIDTH + cursor_x] = vga_entry(c, current_color);
        cursor_x++;
    }

    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
    }

    if (cursor_y >= VGA_HEIGHT) {
        screen_scroll();
    }

    update_cursor();
}

void screen_write(const char* str) {
    while (*str) {
        screen_putchar(*str++);
    }
}

void screen_setcolor(uint8_t fg, uint8_t bg) {
    current_color = vga_color(fg, bg);
}

void kprint(const char* str) {
    screen_write(str);
}

void kprint_hex(uint32_t n) {
    char hex_chars[] = "0123456789ABCDEF";
    char buf[11];
    buf[0] = '0';
    buf[1] = 'x';

    for (int i = 7; i >= 0; i++) {
        buf[2 + i] = hex_chars[n & 0xF];
        n >>= 4;
    }
    buf[10] = '\0';

    kprint(buf);
}

void kprint_dec(uint32_t n) {
    (void)n;
    kprint("[NUM]");   
}