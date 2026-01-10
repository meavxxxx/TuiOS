#include "vmm.h"
#include "pmm.h"
#include "../cpu/isr.h"

static page_directory_t* kernel_directory = 0;
static page_directory_t* current_directory = 0;

static page_table_t* vmm_get_page_table(uint32_t virt, int create) {
    uint32_t pd_index = virt >> 22;

    if (current_directory -> entries[pd_index] & PAGE_PRESENT) {
        return (page_table_t*)(current_directory -> entries[pd_index & 0xFFFFF000]);
    } else if (create) {
        uint32_t phys = pmm_alloc_page();
        if (phys == 0) {
            return 0;
        }

        current_directory -> entries[pd_index] = phys | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;

        page_table_t* table = (page_table_t*)phys;
        for (int i = 0; i < 1024; i++) {
            table -> entries[i] = 0;
        }

        return table;
    }

    return 0;
}

static void page_fault_handler(registers_t* regs) {
    uint32_t faulting_address;
    asm volatile("mov %%cr2, %0" : "=r" (faulting_address));

    int present = regs -> err_code & 0x1;
    int rw = regs -> err_code & 0x2;
    int us = regs -> err_code & 0x4;
    int reserved = regs -> err_code & 0x8;

    extern void kprint(const char*);
    extern void kprint_hex(uint32_t);

    kprint("Page fault! (");
    if (present) kprint("present ");
    if (rw) kprint("write ");
    if (us) kprint("user ");
    if (reserved) kprint("reserved ");
    kprint(") at ");
    kprint_hex(faulting_address);
    kprint("\n");

    for(;;);
}

void vmm_init(void) {
    uint32_t phys = pmm_alloc_page();
    kernel_directory = (page_directory_t*)phys;
    current_directory = kernel_directory;

    for (int i = 0; i < 1024; i++) {
        kernel_directory -> entries[i] = 0;
    }

    for (uint32_t i = 0; i < 0x800000; i += PAGE_SIZE) {
        vmm_map_page(i, i, PAGE_PRESENT | PAGE_WRITE);
    }

    register_interrupt_handler(14, page_fault_handler);

    asm volatile("mov %0, %%cr3" :: "r" (phys));
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r" (cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0" :: "r" (cr0));
}

void vmm_map_page(uint32_t virt, uint32_t phys, uint32_t flags) {
    page_table_t* table = vmm_get_page_table(virt, 1);
    if (table == 0) {
        return;
    }

    uint32_t pt_index = (virt >> 12) & 0x3FF;
    table -> entries[pt_index] = (phys & 0xFFFFF000) | (flags & 0xFFF) | PAGE_PRESENT;
}

void vmm_unmap_page(uint32_t virt) {
    page_table_t* table = vmm_get_page_table(virt, 0);
    if (table == 0) {
        return;
    }

    uint32_t pt_index = (virt >> 12) & 0x3FF;
    table -> entries[pt_index] = 0;

    asm volatile("invlpg (%0)" :: "r" (virt));
}

void vmm_switch_directory(page_directory_t* dir) {
    current_directory = dir;
    asm volatile("mov %0, %%cr3" :: "r" (dir));
}

page_directory_t* vmm_get_directory(void) {
    return current_directory;
}
