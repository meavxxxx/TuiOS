#ifndef IDT_H
#define IDT_H

#include "../libc/stdint.h"

typedef struct idt_entry {
    uint16_t base_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t flags;
    uint16_t base_high;
} __attribute__((packed)) idt_entry_t;

typedef struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idt_ptr_t;

extern idt_ptr_t idt_ptr;

void idt_init(void);

void idt_set_gate(uint8_t num, uint32_t base, uint16_t selector,  uint8_t flags);

#endif