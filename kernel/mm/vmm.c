#include "vmm.h"
#include "pmm.h"
#include "../cpu/isr.h"
#include "../libc/stdint.h"
#include "../libc/string.h"

static page_directory_t* kernel_directory = 0;
static page_directory_t* current_directory = 0;

static page_table_t* vmm_get_page_table(uint32_t virt, int create) {
    uint32_t pd_index = virt >> 22;

    if (current_directory -> entries[pd_index] & PAGE_PRESENT) {
        uint32_t table_phys = current_directory -> entries[pd_index] & 0xFFFFF000;
        return (page_table_t*)table_phys;
    } else if (create) {
        uint32_t phys = pmm_alloc_page();
        if (phys == 0) {
            return 0;
        }

        current_directory -> entries[pd_index] = phys | PAGE_PRESENT | PAGE_WRITE;

        page_table_t* table = (page_table_t*)phys;

        for (int i = 0; i < 1024; i++) {
            table -> entries[i] = 0;
        }

        return table;
    }

    return 0;
}