#include "interrupt.h"
#include "screen.h"
#include "timer.h"
#include "keyboard.h"

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

static const char* exception_messages[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt", 
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",  // 索引13
    "Page Fault",
    "Unknown Exception",
    "x87 Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception"
};

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
    
    // 初始化所有IDT条目为0
    for(int i = 0; i < IDT_ENTRIES; i++) {
        idt_set_gate(i, 0, 0, 0);
    }

    /* 设置中断处理程序 */
    idt_set_gate(0, (uint32_t)isr0, 0x08, 0x8E);   // 除零异常
    idt_set_gate(13, (uint32_t)isr13, 0x08, 0x8E); // 通用保护故障
    
    /* 加载IDT */
    idt_load((uint32_t)&idtp); 
    
    printf("IDT initialized with exception handlers\n");
    // printf("  - ISR0 (Divide Error) at: 0x%x\n", (uint32_t)isr0);
    // printf("  - ISR13 (GPF) at: 0x%x\n", (uint32_t)isr13);
}

void init_pic(void)
{
    outb(0x20, 0x11);
    outb(0xA0, 0x11);

    outb(0x21, 0x20);
    outb(0xA1, 0x28);

    outb(0x21, 0x04);
    outb(0xA1, 0x02);

    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    outb(0x21, 0xFC);
    outb(0xA1, 0xFF);

    // printf("PIC remapped: IRQ0-7 -> 0x20-0x27");
}

void install_timer_interrupt(void)
{
    idt_set_gate(32, (uint32_t)isr32, 0x08, 0x8E);
    printf("Timer interrupt installed at vector 0x20(IRQ0)\n");
}

void install_keyboard_interrupt(void)
{
    idt_set_gate(33, (uint32_t)isr33, 0x08, 0x8E);
    printf("Keyboard interrupt installed at vector 0x21 (IRQ1)\n");
}

/* 默认异常处理 */
void default_exception_handler(struct interrupt_frame* frame) {
    const char* message = "Unknown Exception";
    if(frame->int_no < 20) {
        message = exception_messages[frame->int_no];
    }
    
    printf("\n=== UNHANDLED EXCEPTION %d (%s) ===\n", frame->int_no, message);
    printf("EIP: 0x%x, Error Code: 0x%x\n", frame->eip, frame->err_code);
    printf("CS: 0x%x, EFLAGS: 0x%x\n", frame->cs, frame->eflags);
    printf("System Halted\n");

    // 调试信息：显示一些寄存器值
    printf("Registers - EAX: 0x%x, EBX: 0x%x, ECX: 0x%x\n", 
           frame->eax, frame->ebx, frame->ecx);

    asm volatile("cli");
    while(1) asm volatile("hlt");    
}

/* 除零异常处理程序 */
void divide_by_zero_handler(struct interrupt_frame* frame) {
    printf("\n=== DIVIDE BY ZERO EXCEPTION ===\n");
    printf("Faulting Instruction: 0x%x\n", frame->eip);
    printf("Registers at fault:\n");
    printf("  EAX: 0x%x, EBX: 0x%x, ECX: 0x%x, EDX: 0x%x\n",
           frame->eax, frame->ebx, frame->ecx, frame->edx);
    printf("System Halted\n");
    
    asm volatile("cli");
    while(1) asm volatile("hlt");
}

/* 通用保护故障处理程序 */
void general_protection_fault_handler(struct interrupt_frame* frame) {
    printf("\n=== GENERAL PROTECTION FAULT ===\n");
    printf("Faulting Instruction: 0x%x\n", frame->eip);
    printf("Error Code: 0x%x\n", frame->err_code);
    printf("CS: 0x%x, EFLAGS: 0x%x\n", frame->cs, frame->eflags);
    
    // 错误码详细分析
    printf("Error Code Analysis:\n");
    if(frame->err_code & 0x01) {
        printf("  - External event caused fault\n");
    }
    if(frame->err_code & 0x02) {
        printf("  - Table: %s\n", (frame->err_code & 0x04) ? "LDT" : "GDT");
    }
    printf("  - Segment Index: %d\n", (frame->err_code >> 3) & 0x1FFF);
    
    // 显示更多上下文信息
    printf("Register State:\n");
    printf("  EAX: 0x%x, EBX: 0x%x\n", frame->eax, frame->ebx);
    printf("  ECX: 0x%x, EDX: 0x%x\n", frame->ecx, frame->edx);
    printf("  ESI: 0x%x, EDI: 0x%x\n", frame->esi, frame->edi);
    printf("  EBP: 0x%x, ESP: 0x%x\n", frame->ebp, frame->esp);
    
    printf("System Halted\n");
    
    asm volatile("cli");
    while(1) asm volatile("hlt");
}