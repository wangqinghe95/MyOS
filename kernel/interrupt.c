#include "interrupt.h"
#include "screen.h"

#define IDT_ENTRIES 256

/* IDT条目 */
struct idt_entry {
    uint16_t base_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t flags;
    uint16_t base_high;
} __attribute__((packed));

struct idt_entry idt[IDT_ENTRIES];

/* IDT指针 */
struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

/* 设置IDT门 */
static void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_low = base & 0xFFFF;
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].selector = sel;
    idt[num].zero = 0;
    idt[num].flags = flags;
}

/* 初始化IDT */
void idt_init(void) {
    struct idt_ptr idtp;
    idtp.limit = (sizeof(struct idt_entry) * IDT_ENTRIES) - 1;
    idtp.base = (uint32_t)&idt;
    
    /* 只设置除零异常 */
    idt_set_gate(0, (uint32_t)isr0, 0x08, 0x8E);
    
    /* 加载IDT */
    idt_load((uint32_t)&idtp); 
    
    printf("IDT initialized - Divide by zero handler ready\n");
}

/* 除零异常处理程序 */
void divide_by_zero_handler(struct interrupt_frame* frame) {
    printf("\n=== DIVIDE BY ZERO EXCEPTION ===\n");
    printf("Instruction Pointer: 0x%x\n", frame->eip);
    printf("Code Segment: 0x%x\n", frame->cs);
    printf("Flags: 0x%x\n", frame->eflags);
    printf("Stack Pointer: 0x%x\n", frame->esp);
    printf("=== SYSTEM HALTED ===\n");
    
    while(1) {
        asm volatile("hlt");
    }
}