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

static void page_fault_handler(registers_t* regs) {
    uint32_t faulting_adress;
    asm volatile ("mov %%cr2, %0" : "=r" (faulting_adress));

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
    kprint_hex(faulting_adress);
    kprint("\n");

    for(;;);
}

static uint32_t initial_page_directory[1024] __attribute__((aligned(4096)));
static uint32_t initial_page_table_0[1024] __attribute__((aligned(4096)));
static uint32_t initial_page_table_1[1024] __attribute__((aligned(4096)));

void vmm_init(void) {
    extern void kprint(const char*);

    kprint("VMM: Setting up initial page directory...\n");

    for (int i = 0; i < 1024; i++) {
        initial_page_directory[i] = 0;
    }

    for (int i = 0; i < 1024; i++) {
        initial_page_table_0[i] = (i * 0x1000) | PAGE_PRESENT | PAGE_WRITE;
    }

    for (int i = 0; i < 1024; i++) {
        initial_page_table_1[i] = ((i + 1024) * 0x1000) | PAGE_PRESENT | PAGE_WRITE;
    }

    initial_page_directory[0] = ((uint32_t)initial_page_table_0) | PAGE_PRESENT | PAGE_WRITE;
    initial_page_directory[1] = ((uint32_t)initial_page_table_1) | PAGE_PRESENT | PAGE_WRITE;

    kprint("VMM: Identity mapping complete\n");

    kernel_directory = (page_directory_t*)initial_page_directory;
    current_directory = kernel_directory;

    kprint("VMM: Register page fault handler...\n");
    register_interrupt_handler(14, page_fault_handler);

    kprint("VMM: Enabling paging...\n");

    asm volatile("mov %0, %%cr3" :: "r"(initial_page_directory));

    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r" (cr0));
    cr0 |= 0x80000000;
    cr0 |= 0x10000;
    asm volatile("mov %0, %%cr0" :: "r" (cr0));

    kprint("VMM: Paging enabled!\n");
}

void vmm_map_page(uint32_t virt, uint32_t phys, uint32_t flags) {
    page_table_t* table = vmm_get_page_table(virt, 1);
    if (table == 0) {
        return;
    }

    uint32_t pt_index = (virt >> 12) & 0x3FF;
    table -> entries[pt_index] = (phys & 0xFFFFF000) | (flags & 0xFFF) | PAGE_PRESENT;

    asm volatile("invlpg (%0)" :: "r" (virt) : "memory");
}

void vmm_unmap_page(uint32_t virt) {
    page_table_t* table = vmm_get_page_table(virt, 0);
    if (table == 0) {
        return;
    }

    uint32_t pt_index = (virt >> 12) & 0x3FF;
    table -> entries[pt_index] = 0;

    asm volatile("invlpg (%0)" :: "r" (virt) : "memory");
}

void vmm_switch_directory(page_directory_t* dir) {
    current_directory = dir;
    asm volatile("mov %0, %%cr3" :: "r" (dir));
}

page_directory_t* vmm_get_directory(void) {
    return current_directory;
}