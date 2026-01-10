#include "heap.h"
#include "pmm.h"
#include "vmm.h"

#define HEAP_START 0xC0000000
#define HEAP_INITIAL_SIZE 0x100000
#define HEAP_MAGIC 0x123890AB

typedef struct heap_block {
    uint32_t magic;
    uint32_t size;
    int is_free;
    struct heap_block* next;
} heap_block_t;

static heap_block_t* heap_start = 0;
static uint32_t heap_end = 0;

static uint32_t align_page(uint32_t addr) {
    return (addr + 0xFFF) & 0xFFFFF000;
}

void heap_init(void) {
    heap_start = (heap_block_t*)HEAP_START;
    heap_end = HEAP_START + HEAP_INITIAL_SIZE;

    for (uint32_t i = HEAP_START; i < heap_end; i += PAGE_SIZE) {
        uint32_t phys = pmm_alloc_page();
        vmm_map_page(i, phys, PAGE_PRESENT | PAGE_WRITE);
    }

    heap_start -> magic = HEAP_MAGIC;
    heap_start -> size = HEAP_INITIAL_SIZE - sizeof(heap_block_t);
    heap_start -> is_free = 1;
    heap_start -> next = 0;
}

static void expand_heap(uint32_t size) {
    uint32_t new_end = align_page(heap_end + size);

    for (uint32_t i = heap_end; i < new_end; i += PAGE_SIZE) {
        uint32_t phys = pmm_alloc_page();
        vmm_map_page(i, phys, PAGE_PRESENT | PAGE_WRITE);
    }

    heap_end = new_end;
}

void* kmalloc(uint32_t size) {
    if (size == 0) {
        return 0;
    }

    size = (size + 3) & ~3;

    heap_block_t* current = heap_start;

    while (current) {
        if (current -> magic != HEAP_MAGIC) {
            return 0;
        }

        if (current -> is_free && current -> size >= size) {
            if (current -> size > size + sizeof(heap_block_t) + 16) {
                heap_block_t* new_block = (heap_block_t*)((uint32_t)current + sizeof(heap_block_t) + size);
                new_block -> magic = HEAP_MAGIC;
                new_block -> size = current -> size - size - sizeof(heap_block_t);
                new_block -> is_free = 1;
                new_block -> next = current -> next;

                current -> size = size;
                current -> next = new_block;
            }

            current -> is_free = 0;
            return (void*)((uint32_t)current + sizeof(heap_block_t));
        }

        current = current -> next;
    }

    expand_heap(size + sizeof(heap_block_t));
    return kmalloc(size);
}

void* kmalloc_a(uint32_t size) {
    return kmalloc(size);
}

void* kmalloc_p(uint32_t size, uint32_t* phys) {
    void* addr = kmalloc(size);
    if (phys && addr) {
        *phys = (uint32_t)addr;
    }
    return addr;
}

void kfree(void* ptr) {
    if (!ptr) {
        return;
    }

    heap_block_t* block = (heap_block_t*)((uint32_t)ptr - sizeof(heap_block_t));

    if (block -> magic != HEAP_MAGIC) {
        return;
    }

    block -> is_free = 1;

    if (block -> next && block -> next -> is_free) {
        block -> size += sizeof(heap_block_t) + block -> next -> size;
        block -> next = block -> next -> next;
    }
}