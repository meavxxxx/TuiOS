#include "keyboard.h"
#include "screen.h"
#include "../cpu/isr.h"
#include "../cpu/ports.h"

static const char scancode_to_ascii[] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' '
};

static const char scancode_to_ascii_shift[] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0, ' '
};

volatile char key_buffer[KEY_BUFFER_SIZE];
volatile int key_buffer_read = 0;
volatile int key_buffer_write = 0;
volatile int command_ready = 0;

static char command_buffer[KEY_BUFFER_SIZE];
static int command_index = 0;

static int shift_pressed = 0;
static int ctrl_pressed = 0;
static int alt_pressed = 0;
static int caps_lock = 0;

static void keyboard_callback(registers_t* regs) {
    (void)regs;

    uint8_t scancode = port_byte_in(0x60);

    if (scancode & 0x80) {
        scancode &= 0x7F;

        if (scancode == 0x2A || scancode == 0x36) {
            shift_pressed = 0;
        } else if (scancode == 0x1D) {
            ctrl_pressed = 0;
        } else if (scancode == 0x38) {
            alt_pressed = 0;
        }

        return;
    }

    if (scancode == 0x2A || scancode == 0x36) {
        shift_pressed = 1;
        return;
    } else if (scancode == 0x1D) {
        ctrl_pressed = 1;
        return;
    } else if (scancode == 0x38) {
        alt_pressed = 1;
        return;
    } else if (scancode == 0x3A) {
        caps_lock = !caps_lock;
        return;
    }

    char ascii = 0;
    if (scancode < sizeof(scancode_to_ascii)) {
        if (shift_pressed || caps_lock) {
            ascii = scancode_to_ascii_shift[scancode];
        } else {
            ascii = scancode_to_ascii[scancode];
        }
    }

    if (ascii == 0) {
        return;
    }

    if (ascii == '\n') {
        screen_putchar('\n');
        command_buffer[command_index] = '\0';
        command_ready = 1;
        return;
    } else if (ascii == '\b') {
        if (command_index > 0) {
            command_index--;
            command_buffer[command_index] = '\0';
            screen_putchar('\b');
        }
        return;
    }

    if (command_index < KEY_BUFFER_SIZE - 1) {
        command_buffer[command_index++] = ascii;
        screen_putchar(ascii);
    }
}

void keyboard_init(void) {
    register_interrupt_handler(33, keyboard_callback);

    key_buffer_read = 0;
    key_buffer_write = 0;
    command_index = 0;
    command_ready = 0;
}

char keyboard_getchar(void) {
    while (key_buffer_read == key_buffer_write) {
        asm volatile("hlt");
    }

    char c = key_buffer[key_buffer_read];
    key_buffer_read = (key_buffer_read + 1) % KEY_BUFFER_SIZE;
    return c;
}

int keyboard_available(void) {
    return key_buffer_read != key_buffer_write;
}

int keyboard_get_command(char* buffer, int max_len) {
    if (!command_ready) {
        return 0;
    }

    int len = 0;
    while (command_buffer[len] != '\0' && len < max_len - 1) {
        buffer[len] = command_buffer[len];
        len++;
    }
    buffer[len] = '\0';

    command_index = 0;
    command_buffer[0] = '\0';
    command_ready = 0;

    return len;
}