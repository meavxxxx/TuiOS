# TuiOS - A POSIX-Compatible Operating System

TuiOS is a 32-bit x86 operating system written from scratch with the goal of running GNU software.

## Features

- **32-bit Protected Mode** x86 architecture
- **Multiboot** bootloader compatible (GRUB)
- **Memory Management**
  - Physical Memory Manager (PMM)
  - Virtual Memory Manager with Paging
  - Kernel Heap (kmalloc/kfree)
- **Interrupt Handling**
  - IDT and ISR setup
  - IRQ handling with PIC remapping
- **Drivers**
  - VGA text mode
  - PS/2 keyboard
  - PIT timer
- **Standard Library** (libc subset)

## Building

### Prerequisites

```bash
# Ubuntu/Debian
sudo apt-get install build-essential nasm qemu-system-x86 grub-pc-bin xorriso mtools

# Arch Linux
sudo pacman -S base-devel nasm qemu grub xorriso mtools
```

### Compilation

```bash
# Build the kernel
make

# Run in QEMU
make run

# Debug in QEMU (waits for GDB connection on port 1234)
make debug

# Clean build files
make clean
```

## Project Structure

```
TuiOS/
├── kernel/
│   ├── boot.asm          # Bootloader entry point
│   ├── kernel.c          # Main kernel
│   ├── cpu/              # CPU-specific code
│   │   ├── gdt.c/h       # Global Descriptor Table
│   │   ├── idt.c/h       # Interrupt Descriptor Table
│   │   ├── isr.c/h/asm   # Interrupt Service Routines
│   │   └── ports.h       # Port I/O
│   ├── drivers/          # Device drivers
│   │   ├── screen.c/h    # VGA text mode
│   │   ├── keyboard.c/h  # PS/2 keyboard
│   │   └── timer.c/h     # PIT timer
│   ├── mm/               # Memory management
│   │   ├── pmm.c/h       # Physical memory
│   │   ├── vmm.c/h       # Virtual memory (paging)
│   │   └── heap.c/h      # Kernel heap
│   └── libc/             # Standard library
│       ├── stdint.h
│       ├── stddef.h
│       └── string.c/h
├── Makefile              # Build system
├── linker.ld             # Linker script
└── README.md             # This file
```

## Roadmap to POSIX Compatibility

### Phase 1: Basic Infrastructure ✓
- [x] Bootloader (GRUB multiboot)
- [x] GDT, IDT, ISR/IRQ
- [x] Physical memory manager
- [x] Virtual memory (paging)
- [x] Kernel heap
- [x] Basic drivers (VGA, keyboard, timer)

### Phase 2: File System (In Progress)
- [ ] VFS (Virtual File System) layer
- [ ] ext2 filesystem support
- [ ] devfs for device files
- [ ] Basic file operations (open, read, write, close)

### Phase 3: Process Management
- [ ] Process structure
- [ ] Context switching
- [ ] Round-robin scheduler
- [ ] ELF binary loader
- [ ] User mode support

### Phase 4: System Calls
- [ ] Syscall interface
- [ ] Basic POSIX syscalls:
  - fork, exec, exit, wait
  - open, read, write, close
  - brk, mmap, munmap

### Phase 5: IPC and Signals
- [ ] Pipes
- [ ] Signals
- [ ] Message queues

### Phase 6: POSIX libc
- [ ] Port newlib or musl
- [ ] Implement POSIX functions
- [ ] Create cross-compiler

### Phase 7: Userspace
- [ ] init process
- [ ] Basic shell
- [ ] Port GNU coreutils
- [ ] Port bash

## Development Notes

### Memory Map

```
0x00000000 - 0x000FFFFF : Reserved (BIOS, VGA)
0x00100000 - 0x003FFFFF : Kernel code/data (~3MB)
0x00400000 - ...        : Physical memory allocator
0xC0000000 - ...        : Kernel heap (grows dynamically)
```

### Debugging

To debug with GDB:

```bash
# Terminal 1
make debug

# Terminal 2
gdb build/kernel.bin
(gdb) target remote localhost:1234
(gdb) break kmain
(gdb) continue
```

### Adding New Features

1. Create feature files in appropriate directory
2. Update Makefile if needed
3. Include headers in kernel.c
4. Initialize in kmain()

## Resources

- [OSDev Wiki](https://wiki.osdev.org/)
- [JamesM's Kernel Tutorial](https://web.archive.org/web/20160326064709/http://jamesmolloy.co.uk/tutorial_html/)
- [xv6 Operating System](https://pdos.csail.mit.edu/6.828/2012/xv6.html)
- [Intel x86 Manual](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html)

## License

MIT License - Feel free to use and modify!

## Contributing

This is a learning project, but contributions are welcome!

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Submit a pull request

## Author

Created as a learning project to understand operating system internals.
