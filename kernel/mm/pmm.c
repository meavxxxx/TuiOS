#include "pmm.h"

static uint32_t* page_bitmap = (uint32_t*)0x10000;
static uint32_t total_pages = 0;
static uint32_t used_pages = 0;

static inline void bitmap_set(uint32_t page) {
    uint32_t index = page / 32;
    uint32_t bit = page % 32;
    page_bitmap[index] |= (1 << bit);
}

static inline void bitmap_clear(uint32_t page) {
    uint32_t index = page / 32;
    uint32_t bit = page % 32;
    page_bitmap[index] &= ~(1 << bit);
}

static inline int bitmap_test(uint32_t page) {
    uint32_t index = page / 32;
    uint32_t bit = page % 32;
    return page_bitmap[index] & (1 << bit);
}

void pmm_init(uint32_t mem_size) {
    total_pages = mem_size / PAGE_SIZE;
    used_pages = 0;

    uint32_t bitmap_size = total_pages / 32 + 1;
    for (uint32_t i = 0; i < bitmap_size; i++) {
        page_bitmap[i] = 0;
    }

    uint32_t kernel_pages = (0x100000 + 0x400000) / PAGE_SIZE;

    for (uint32_t i = 0; i < kernel_pages; i++) {
        bitmap_set(i);
        used_pages++;
    }
}

uint32_t pmm_alloc_page(void) {
    for (uint32_t i = 0; i < total_pages; i++) {
        if(!bitmap_test(i)) {
            bitmap_set(i);
            used_pages++;
            return i * PAGE_SIZE;
        }
    }
    return 0;
}

void pmm_free_page(uint32_t page) {
    uint32_t page_num = page / PAGE_SIZE;
    if (bitmap_test(page_num)) {
        bitmap_clear(page_num);
        used_pages--;
    }
}

uint32_t pmm_get_total_memory(void) {
    return total_pages * PAGE_SIZE;
}

uint32_t pmm_get_free_memory(void) {
    return (total_pages - used_pages) * PAGE_SIZE;
}