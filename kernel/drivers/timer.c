#include "timer.h"
#include "../cpu/isr.h"
#include "../cpu/ports.h"

static volatile uint32_t tick_count = 0;

static void timer_callback(registers_t* regs) {
    (void)regs;
    tick_count++;
}

void timer_init(uint32_t frequency) {
    register_interrupt_handler(32, timer_callback);

    uint32_t divisor = 1193180;

    port_byte_out(0x43, 0x36);

    uint8_t low = (uint8_t)(divisor & 0xFF);
    uint8_t high = (uint8_t)((divisor >> 8) & 0xFF);

    port_byte_out(0x40, low);
    port_byte_out(0x40, high);
}

uint32_t timer_get_ticks(void) {
    return tick_count;
}

void timer_wait(uint32_t ticks) {
    uint32_t end_tick = tick_count + ticks;
    while(tick_count < end_tick) {
        asm_volatile("hlt");
    }
}