MULTIBOOT_MAGIC equ 0x1BADB002
MULTIBOOT_ALIGN equ 1 << 0
MULTIBOOT_MEMINFO equ 1 << 1
MULTIBOOT_FLAGS equ MULTIBOOT_ALIGN | MULTIBOOT_MEMINFO
MULTIBOOT_CHECKSUM equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

section .multiboot
align 4
    dd MULTIBOOT_MAGIC
    dd MULTIBOOT_FLAGS
    dd MULTIBOOT_CHECKSUM

section .bss
align 16
stack_bottom:
    resb 16384


section .text
global start
extern kmain

start:
    mov esp, stack_bottom + 16384

    push ebx
    push eax

    call kmain
    cli

.hang:
    hlt
    jmp .hang

global gdt_flush
extern gdt_ptr


gdt_flush:
    lgdt [gdt_ptr]
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:.flush
.flush:
    ret

global idt_flush
extern idt_ptr

idt_flush:
    lidt [idt_ptr]
    ret

global tss_flush

tss_flush:
    mov ax, 0x2B
    ltr ax
    ret