#ifndef TIMER_H
#define TIMER_H

#include "types.h"

#define PIT_CHANNEL0_PORT 0x40
#define PIT_COMMAND_PORT 0x43
#define TIMER_FREQUENCY 100

void init_timer(void);
uint32_t get_ticks(void);
void timer_interrupt_handler(void);

#endif