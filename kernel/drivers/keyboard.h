#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

#define KEY_BUFFER_SIZE 256

extern volatile char key_buffer[KEY_BUFFER_SIZE];
extern volatile int key_buffer_read;
extern volatile int key_buffer_write;
extern volatile int command_ready;

void keyboard_init(void);
char keyboard_getchar(void);
int keyboard_available(void);
int keyboard_get_command(char* buffer, int max_len);

#endif