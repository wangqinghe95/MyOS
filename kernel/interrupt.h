#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "types.h"

struct interrupt_frame {
    uint32_t eip, cs, eflags, esp, ss;
    uint32_t error_code;
};

void idt_init(void);
void divide_by_zero_handler(struct interrupt_frame* frame);
void test_interrupt(void);

extern void isr0(void);
extern void idt_load(uint32_t idt_ptr);

#endif