#include "drivers/screen.h"
#include "cpu/gdt.h"
#include "cpu/idt.h"
#include "cpu/isr.h"
#include "drivers/keyboard.h"
#include "drivers/timer.h"
#include "mm/pmm.h"
#include "mm/vmm.h"
#include "mm/heap.h"

typedef struct multiboot_info {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t syms[4];
    uint32_t mmap_length;
    uint32_t mmap_addr;
} __atribute__((packed)) multiboot_info_t;

void kmain(uint32_t magic, multiboot_info_t* mboot) {
    screen_init();
    screen_clear();

    kprint("TuiOS Kernel Starting...\n")
    kprint("=========================\n\n");

    if (magic != 0x2BADB002) {
        kprint("ERROR: Invalid multiboot magic number!\n");
        for(;;);
    }
    kprint("[OK] Multiboot verified\n");

    gdt_init();
    kprint("[OK] GDT initialized\n");

    idt_init();
    kprint("[OK] IDT initialized\n");

    isr_init();
    kprint("[OK] ISR initialized\n");

    irq_init();
    kprint("[OK] IRQ initialized\n");

    asm volatile("sti");
    kprint("[OK] Interrupts enabled\n");

    time_init(100);
    kprint("[OK] Timer initialized\n");

    uint32_t total_mem = (mboot -> mem_lower + mboot -> mem_upper) * 1024;
    pmm_init(total_mem);
    kprint("[OK] Physical memory manager initialized\n");

    vmm_init();
    kprint("[OK] Virtual memory manager initialized\n");

    heap_init();
    kprint("[OK] Keyboard initialized\n");

    kprint("\n========================\n");
    kprint("TuiOS Ready!\n");
    kprint("Type 'help' for available commands\n\n");
    kprint("TuiOS> ");

    for(;;) {
        asm volatile("hlt");
    }
}