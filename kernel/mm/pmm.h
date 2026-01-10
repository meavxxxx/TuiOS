#ifndef PMM_H
#define PMM_H

#include <stdint.h>

#define PAGE_SIZE 4096
#define PAGES_PER_BYTE 8

void pmm_init(uint32_t mem_size);
uint32_t pmm_alloc_page(void);
void pmm_free_page(uint32_t page);
uint32_t pmm_get_total_memory(void);
uint32_t pmm_get_free_memory(void);

#endif