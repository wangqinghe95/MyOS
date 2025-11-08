#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "types.h"

/* 中断栈帧结构体 - 必须与汇编代码栈布局匹配 */
struct interrupt_frame {
    uint32_t gs, fs, es, ds;                    // 软件保存的段寄存器
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // pusha顺序
    uint32_t int_no, err_code;                  // 中断信息
    uint32_t eip, cs, eflags, user_esp, ss;     // CPU自动保存
};

/* 函数声明 */
void idt_init(void);
void idt_load(uint32_t idt_ptr);

/* 汇编函数声明 */
extern void isr0(void);
extern void isr13(void);

/* 异常处理函数 */
void divide_by_zero_handler(struct interrupt_frame* frame);
void general_protection_fault_handler(struct interrupt_frame* frame);
void default_exception_handler(struct interrupt_frame* frame);

#endif