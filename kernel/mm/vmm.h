#ifndef VMM_H
#define VMM_H

#include "../libc/stdint.h"

#define PAGE_PRESENT 0x1
#define PAGE_WRITE 0x2
#define PAGE_USER 0x4

typedef uint32_t pde_t;
typedef uint32_t pte_t;

typedef struct page_directory {
    pde_t entries[1024];
} page_directory_t;

typedef struct page_table {
    pte_t entries[1024];
} page_table_t;

void vmm_init(void);
void vmm_map_page(uint32_t virt, uint32_t phys, uint32_t flags);
void vmm_unmap_page(uint32_t virt);
void vmm_switch_directory(page_directory_t* dir);
page_directory_t* vmm_get_directory(void);

#endif