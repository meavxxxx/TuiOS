#include "isr.h"
#include "idt.h"
#include "ports.h"
#include "../drivers/screen.h"

isr_handler_t interrupt_handlers[256];

static const char* exception_messages[] = {
    "Division BY Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bound",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",
    "Coprocessor Segmen Overrun",
    "Bad TSS",
    "Segmen Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unkown Interrupt",
    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
};

void isr_init(void) {
    idt_set_gate(0, (uint32_t)isr0, 0x08, 0x8E);
    idt_set_gate(1, (uint32_t)isr1, 0x08, 0x8E);
    idt_set_gate(2, (uint32_t)isr2, 0x08, 0x8E);
    idt_set_gate(3, (uint32_t)isr3, 0x08, 0x8E);
    idt_set_gate(4, (uint32_t)isr4, 0x08, 0x8E);
    idt_set_gate(5, (uint32_t)isr5, 0x08, 0x8E);
    idt_set_gate(6, (uint32_t)isr6, 0x08, 0x8E);
    idt_set_gate(7, (uint32_t)isr7, 0x08, 0x8E);
    idt_set_gate(8, (uint32_t)isr8, 0x08, 0x8E);
    idt_set_gate(9, (uint32_t)isr9, 0x08, 0x8E);
    idt_set_gate(10, (uint32_t)isr10, 0x08, 0x8E);
    idt_set_gate(11, (uint32_t)isr11, 0x08, 0x8E);
    idt_set_gate(12, (uint32_t)isr12, 0x08, 0x8E);
    idt_set_gate(13, (uint32_t)isr13, 0x08, 0x8E);
    idt_set_gate(14, (uint32_t)isr14, 0x08, 0x8E);
    idt_set_gate(15, (uint32_t)isr15, 0x08, 0x8E);
    idt_set_gate(16, (uint32_t)isr16, 0x08, 0x8E);
    idt_set_gate(17, (uint32_t)isr17, 0x08, 0x8E);
    idt_set_gate(18, (uint32_t)isr18, 0x08, 0x8E);
    idt_set_gate(19, (uint32_t)isr19, 0x08, 0x8E);
    idt_set_gate(20, (uint32_t)isr20, 0x08, 0x8E);
    idt_set_gate(21, (uint32_t)isr21, 0x08, 0x8E);
    idt_set_gate(22, (uint32_t)isr22, 0x08, 0x8E);
    idt_set_gate(23, (uint32_t)isr23, 0x08, 0x8E);
    idt_set_gate(24, (uint32_t)isr24, 0x08, 0x8E);
    idt_set_gate(25, (uint32_t)isr25, 0x08, 0x8E);
    idt_set_gate(26, (uint32_t)isr26, 0x08, 0x8E);
    idt_set_gate(27, (uint32_t)isr27, 0x08, 0x8E);
    idt_set_gate(28, (uint32_t)isr28, 0x08, 0x8E);
    idt_set_gate(29, (uint32_t)isr29, 0x08, 0x8E);
    idt_set_gate(30, (uint32_t)isr30, 0x08, 0x8E);
    idt_set_gate(31, (uint32_t)isr31, 0x08, 0x8E);
};

static void pic_remap(void) {
    
    port_byte_out(0x20, 0x11);
    port_byte_out(0xA0, 0x11);
    
    
    port_byte_out(0x21, 0x20);
    port_byte_out(0xA1, 0x28);
    
    
    port_byte_out(0x21, 0x04);
    port_byte_out(0xA1, 0x02);
    
    
    port_byte_out(0x21, 0x01);
    port_byte_out(0xA1, 0x01);
    
    
    port_byte_out(0x21, 0x00);
    port_byte_out(0xA1, 0x00);
}

void irq_init(void) {
    pic_remap();
    
    idt_set_gate(32, (uint32_t)irq0, 0x08, 0x8E);
    idt_set_gate(33, (uint32_t)irq1, 0x08, 0x8E);
    idt_set_gate(34, (uint32_t)irq2, 0x08, 0x8E);
    idt_set_gate(35, (uint32_t)irq3, 0x08, 0x8E);
    idt_set_gate(36, (uint32_t)irq4, 0x08, 0x8E);
    idt_set_gate(37, (uint32_t)irq5, 0x08, 0x8E);
    idt_set_gate(38, (uint32_t)irq6, 0x08, 0x8E);
    idt_set_gate(39, (uint32_t)irq7, 0x08, 0x8E);
    idt_set_gate(40, (uint32_t)irq8, 0x08, 0x8E);
    idt_set_gate(41, (uint32_t)irq9, 0x08, 0x8E);
    idt_set_gate(42, (uint32_t)irq10, 0x08, 0x8E);
    idt_set_gate(43, (uint32_t)irq11, 0x08, 0x8E);
    idt_set_gate(44, (uint32_t)irq12, 0x08, 0x8E);
    idt_set_gate(45, (uint32_t)irq13, 0x08, 0x8E);
    idt_set_gate(46, (uint32_t)irq14, 0x08, 0x8E);
    idt_set_gate(47, (uint32_t)irq15, 0x08, 0x8E);
}

void register_interrupt_handler(uint8_t n, isr_handler_t handler) {
    interrupt_handlers[n] = handler;
}

void isr_handler(registers_t* regs) {
    extern void kprint(const char*);
    extern void kprint_dec(uint32_t);

    if (interrupt_handlers[regs -> int_no] != 0) {
        isr_handler_t handler = interrupt_handlers[regs -> int_no];
        handler(regs);
    } else {
        kprint("Unhandled exception #");
        kprint_dec(regs -> int_no);
        kprint("\n");

        if (regs -> int_no < 23) {
            kprint(exception_messages[regs -> int_no]);
            kprint("\n");
        }

        kprint("EIP: ");
        kprint_dec(regs -> eip);
        kprint(" CS: ");
        kprint_dec(regs -> cs);
        kprint(" EFLAGS: ");
        kprint_dec(regs -> eflags);
        kprint("\n");

        for(;;) {
            asm volatile("hlt");
        }
    }
}
void irq_handler(registers_t* regs) {
    if (regs -> int_no >= 40) {
        port_byte_out(0xA0, 0x20);
    }
    port_byte_out(0x20, 0x20);

    if (interrupt_handlers[regs -> int_no] != 0) {
        isr_handler_t handler = interrupt_handlers[regs -> int_no];
        handler(regs);
    }
}