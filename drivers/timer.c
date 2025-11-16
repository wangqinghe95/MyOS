#include "timer.h"
#include "screen.h"

volatile uint32_t system_ticks = 0;

void init_timer(void)
{
    uint32_t divisor = 1193180 / TIMER_FREQUENCY;

    outb(PIT_COMMAND_PORT, 0x36);

    outb(PIT_CHANNEL0_PORT, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0_PORT, (uint8_t)((divisor >> 8) & 0xFF));

    printf("PIT TImer initialized at %d Hz\n", TIMER_FREQUENCY);
}

void timer_interrupt_handler(void)
{
    system_ticks++;

    if(system_ticks % TIMER_FREQUENCY == 0) {
        // printf("System uptiem: %d seconds\n", system_ticks/TIMER_FREQUENCY);
    }

    outb(0x20, 0x20);
}

uint32_t get_ticks(void)
{
    return system_ticks;
}