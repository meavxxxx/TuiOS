#ifndef HEAP_H
#define HEAP_H

#include "../libc/stdint.h"

void heap_init(void);
void* kmalloc(uint32_t size);
void* kmalloc_a(uint32_t size);
void* kmalloc_p(uint32_t size, uint32_t* phys);
void* kmalloc_ap(uint32_t size, uint32_t* phys);
void kfree(void* ptr);

#endif