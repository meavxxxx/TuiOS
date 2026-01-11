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
        // Преобразуем физический адрес в виртуальный (предполагая identity mapping)
        uint32_t table_phys = current_directory -> entries[pd_index] & 0xFFFFF000;
        return (page_table_t*)table_phys;
    } else if (create) {
        uint32_t phys = pmm_alloc_page();
        if (phys == 0) {
            return 0;
        }

        current_directory -> entries[pd_index] = phys | PAGE_PRESENT | PAGE_WRITE;

        // Преобразуем физический адрес в виртуальный (при identity mapping физ. адрес = вирт. адресу)
        page_table_t* table = (page_table_t*)phys;
        
        // Убедимся, что таблица страниц находится в памяти, добавив барьер
        asm volatile("" ::: "memory");
        
        for (int i = 0; i < 1024; i++) {
            table -> entries[i] = 0;
        }

        // Синхронизируем кэш
        asm volatile("" ::: "memory");

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

// Глобальная статическая структура для начального page directory
static uint32_t initial_page_directory[1024] __attribute__((aligned(4096)));

void vmm_init(void) {
    extern void kprint(const char*);
    extern void kprint_hex(uint32_t);

    kprint("VMM: Setting up initial page directory...\n");

    // Используем заранее выделенную структуру как начальный page directory
    uint32_t phys = (uint32_t)initial_page_directory;
    
    kprint("VMM: Initial page directory at: ");
    //kprint_hex(phys);
    kprint("\n");

    // Инициализируем начальный каталог страниц
    for (int i = 0; i < 1024; i++) {
        initial_page_directory[i] = 0;
    }

    kprint("VMM: Identity mapping first 8MB...\n");

    // Identity mapping первых 8MB (0x0 - 0x800000)
    for (uint32_t i = 0; i < 0x800000; i += 0x1000) {
        uint32_t table_index = i >> 22; // Номер директории
        uint32_t page_index = (i >> 12) & 0x3FF; // Номер страницы в таблице
        
        // Создаем таблицу страниц, если она не существует
        if (!(initial_page_directory[table_index] & PAGE_PRESENT)) {
            uint32_t table_phys = pmm_alloc_page();
            if (!table_phys) {
                kprint("VMM: Failed to allocate page table!\n");
                for(;;);
            }
            
            // Настройка записи каталога страниц
            initial_page_directory[table_index] = table_phys | PAGE_PRESENT | PAGE_WRITE;
            
            // Очищаем таблицу страниц
            uint32_t* table = (uint32_t*)table_phys;
            for (int j = 0; j < 1024; j++) {
                table[j] = 0;
            }
        }
        
        // Получаем адрес таблицы страниц
        uint32_t* table = (uint32_t*)(initial_page_directory[table_index] & 0xFFFFF000);
        // Устанавливаем отображение страницы (identity mapping: virt == phys)
        table[page_index] = i | PAGE_PRESENT | PAGE_WRITE;
    }

    kprint("VMM: Identity mapping complete\n");

    // Устанавливаем глобальные переменные
    kernel_directory = (page_directory_t*)phys;
    current_directory = kernel_directory;

    kprint("VMM: Registering page fault handler...\n");

    register_interrupt_handler(14, page_fault_handler);

    kprint("VMM: Enabling paging...\n");

    // Загружаем физический адрес директории страниц в CR3
    asm volatile("mov %0, %%cr3" :: "r" (phys));

    // Включаем страничную память
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r" (cr0));
    cr0 |= 0x80000000;  // Устанавливаем бит PG (Paging)
    cr0 |= 0x10000;     // Устанавливаем бит WP (Write Protect)
    asm volatile("mov %0, %%cr0" :: "r" (cr0));

    kprint("VMM: Paging enabled!\n");
    
    // Убедимся, что указатель на текущую директорию обновлен
    current_directory = kernel_directory;
}

void vmm_map_page(uint32_t virt, uint32_t phys, uint32_t flags) {
    page_table_t* table = vmm_get_page_table(virt, 1);
    if (table == 0) {
        return;
    }

    uint32_t pt_index = (virt >> 12) & 0x3FF;
    table -> entries[pt_index] = (phys & 0xFFFFF000) | (flags & 0xFFF) | PAGE_PRESENT;
    
    // Инвалидируем кэш TLB для отображаемой страницы
    asm volatile("invlpg (%0)" :: "r" (virt));
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
    // При записи в CR3 должен использоваться физический адрес
    // Так как мы используем identity mapping, физический адрес равен виртуальному
    asm volatile("mov %0, %%cr3" :: "r" (dir));
    asm volatile("invlpg (%0)" :: "r" (dir)); // Инвалидируем кэш TLB для надежности
}

page_directory_t* vmm_get_directory(void) {
    return current_directory;
}
