section .text

extern divide_by_zero_handler

global idt_load
global isr0

idt_load:
    mov eax, [esp + 4]
    lidt [eax]
    ret

isr0:
    cli
    ; 保存寄存器
    pusha
    ; 创建中断帧
    push esp
    call divide_by_zero_handler
    ; 不应该返回
    add esp, 4
    popa
    sti
    iret