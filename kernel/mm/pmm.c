#include "pmm.h"

static uint32_t* page_bitmap = (uint32_t*)0x10000;
static uint32_t total_pages = 0;
static uint32_t used_pages = 0;

static inline void bitmap_set(uint32_t page) {
    uint32_t index = page >> 5;
    uint32_t bit = page & 31;
    page_bitmap[index] |= (1U << bit);
}

static inline void bitmap_clear(uint32_t page) {
    uint32_t index = page >> 5;
    uint32_t bit = page & 31;
    page_bitmap[index] &= ~(1U << bit);
}

static inline int bitmap_test(uint32_t page) {
    uint32_t index = page >> 5;
    uint32_t bit = page & 31;
    return page_bitmap[index] & (1U << bit);
}

void pmm_init(uint32_t mem_size) {
    total_pages = mem_size >> 12;
    used_pages = 0;

    uint32_t bitmap_size = (total_pages >> 5) + 1;
    for (uint32_t i = 0; i < bitmap_size; i++) {
        page_bitmap[i] = 0;
    }

    // Зарезервируем страницы под ядро и область битовой карты
    uint32_t kernel_pages = (0x100000 + 0x400000) >> 12;
    
    // Резервируем также память под саму битовую карту
    uint32_t bitmap_pages = ((bitmap_size << 2) + 0xFFF) >> 12;
    uint32_t bitmap_start_page = ((uint32_t)page_bitmap) >> 12;
    
    for (uint32_t i = 0; i < kernel_pages; i++) {
        if (i < total_pages) {
            bitmap_set(i);
            used_pages++;
        }
    }
    
    // Резервируем страницы под битовую карту
    for (uint32_t i = bitmap_start_page; i < bitmap_start_page + bitmap_pages; i++) {
        if (i < total_pages) {
            bitmap_set(i);
            used_pages++;
        }
    }
}

uint32_t pmm_alloc_page(void) {
    for (uint32_t i = 0; i < total_pages; i++) {
        if(!bitmap_test(i)) {
            bitmap_set(i);
            used_pages++;
            return i << 12;
        }
    }
    return 0;
}

void pmm_free_page(uint32_t page) {
    uint32_t page_num = page >> 12;
    if (bitmap_test(page_num)) {
        bitmap_clear(page_num);
        used_pages--;
    }
}

uint32_t pmm_get_total_memory(void) {
    return total_pages << 12;
}

uint32_t pmm_get_free_memory(void) {
    return (total_pages - used_pages) << 12;
}