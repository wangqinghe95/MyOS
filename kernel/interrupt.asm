section .text

; 外部C函数
extern divide_by_zero_handler

; 全局符号
global idt_load
global isr0

; 加载IDT
idt_load:
    mov eax, [esp + 4]  ; 获取IDT指针
    lidt [eax]          ; 加载IDT
    ret

; 除零异常处理程序
isr0:
    cli                 ; 关中断
    
    ; 保存寄存器
    pusha
    
    ; 调用C处理程序
    push esp
    call divide_by_zero_handler
    
    ; 不会返回这里
    add esp, 4
    popa
    sti
    iret