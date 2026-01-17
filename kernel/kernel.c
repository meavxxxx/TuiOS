#include "drivers/screen.h"
#include "cpu/gdt.h"
#include "cpu/idt.h"
#include "cpu/isr.h"
#include "drivers/keyboard.h"
#include "drivers/timer.h"
#include "mm/pmm.h"
#include "mm/vmm.h"
#include "mm/heap.h"
#include "libc/stdint.h"
#include "libc/string.h"

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
} __attribute__((packed)) multiboot_info_t;

void kmain(uint32_t magic, multiboot_info_t* mboot) {
    screen_init();
    screen_clear();

    kprint("TuiOS Kernel Starting...\n");
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

    uint32_t total_mem = (mboot->mem_lower + mboot->mem_upper) * 1024;
    pmm_init(total_mem);
    kprint("[OK] Physical memory manager initialized\n");

    vmm_init();
    kprint("[OK] Virtual memory manager initialized\n");

    heap_init();
    kprint("[OK] Heap initialized\n");

    keyboard_init();
    kprint("[OK] Keyboard initialized\n");

    timer_init(100);
    kprint("[OK] Timer initialized\n");

    asm volatile("sti");
    kprint("[OK] Interrupts enabled\n");

    kprint("\n========================\n");
    kprint("TuiOS Ready!\n");
    kprint("Type 'help' for available commands\n\n");
    kprint("TuiOS> ");

    char cmd[256];
    
    for(;;) {
        if (keyboard_get_command(cmd, 256)) {
            if (cmd[0] == '\0') {
                kprint("TuiOS> ");
            } else if (strcmp(cmd, "help") == 0) {
                kprint("Available commands:\n");
                kprint("  help    - Show this help\n");
                kprint("  clear   - Clear screen\n");
                kprint("  hello   - Print hello message\n");
                kprint("  mem     - Show memory info\n");
                kprint("TuiOS> ");
            } else if (strcmp(cmd, "clear") == 0) {
                screen_clear();
                kprint("TuiOS> ");
            } else if (strcmp(cmd, "hello") == 0) {
                kprint("Hello from TuiOS!\n");
                kprint("TuiOS> ");
            } else if (strcmp(cmd, "mem") == 0) {
                kprint("Total memory: ");
                kprint_dec(pmm_get_total_memory() / 1024);
                kprint(" KB\n");
                kprint("Free memory: ");
                kprint_dec(pmm_get_free_memory() / 1024);
                kprint(" KB\n");
                kprint("TuiOS> ");
            } else {
                kprint("Unknown command: ");
                kprint(cmd);
                kprint("\n");
                kprint("TuiOS> ");
            }
        }
        
        asm volatile("hlt");
    }
}