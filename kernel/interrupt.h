#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <stdint.h>

/* 中断帧 */
struct interrupt_frame {
    uint32_t eip;       // 指令指针
    uint32_t cs;        // 代码段
    uint32_t eflags;    // 标志寄存器
    uint32_t esp;       // 栈指针  
    uint32_t ss;        // 栈段
};

/* 函数声明 */
void idt_init(void);
void divide_by_zero_handler(struct interrupt_frame* frame);

/* 外部汇编函数 */
extern void idt_load(uint32_t idt_ptr);
extern void isr0(void);  // 除零异常

#endif