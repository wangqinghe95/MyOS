#include "interrupt.h"
#include "screen.h"

#define IDT_ENTRIES 256

struct idt_entry {
    uint16_t base_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t flags;
    uint16_t base_high;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

static struct idt_entry idt[IDT_ENTRIES];

static void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_low = base & 0xFFFF;
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].selector = sel;
    idt[num].zero = 0;
    idt[num].flags = flags;
}

void idt_init(void) {
    struct idt_ptr idtp;
    idtp.limit = (sizeof(struct idt_entry) * IDT_ENTRIES) - 1;
    idtp.base = (uint32_t)&idt;

    // 设置除零异常
    idt_set_gate(0, (uint32_t)isr0, 0x08, 0x8E);

    idt_load((uint32_t)&idtp);
    printf("IDT initialized with divide-by-zero handler\n");
}

void divide_by_zero_handler(struct interrupt_frame* frame) {
    printf("\n=== DIVIDE BY ZERO EXCEPTION ===\n");
    printf("Instruction Pointer: 0x%x\n", frame->eip);
    printf("Code Segment: 0x%x\n", frame->cs);
    printf("Stack Pointer: 0x%x\n", frame->esp);
    printf("=== SYSTEM HALTED ===\n");
    
    // 彻底挂起
    asm volatile("cli");
    while(1) {
        asm volatile("hlt");
    }
}

void test_interrupt(void) {
    printf("Testing interrupt system...\n");
    printf("About to trigger divide by zero...\n");
    
    for (volatile int i = 0; i < 1000000; i++);
    
    asm volatile("mov $0, %eax");
    asm volatile("div %eax");
    
    printf("ERROR: Should not reach here!\n");
    while(1);
}