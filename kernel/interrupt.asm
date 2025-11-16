section .text

; 外部C函数
extern divide_by_zero_handler
extern general_protection_fault_handler
extern default_exception_handler
extern timer_interrupt_handler
extern keyboard_interrupt_handler

; 全局符号
global idt_load
global isr0, isr13, isr32, irs33

; 加载IDT
idt_load:
    mov eax, [esp + 4]  ; 获取IDT指针
    lidt [eax]          ; 加载IDT
    ret

%macro ISR_NOERRCODE 1
global isr%1
isr%1:
    cli
    push byte 0         ; 压入伪错误码
    push byte %1        ; 压入中断号
    jmp isr_common
%endmacro

%macro ISR_ERRCODE 1
global isr%1
isr%1:
    cli
    push byte %1        ; 压入中断号 (CPU已压入错误码)
    jmp isr_common
%endmacro

; 定义具体的中断处理程序
ISR_NOERRCODE 0    ; 除零异常
ISR_ERRCODE 13     ; 通用保护故障
ISR_NOERRCODE 32    ; 定时器中断（IRQ0）
ISR_NOERRCODE 33

; 通用中断处理程序
isr_common:
    ; 保存所有寄存器 - 顺序必须与C结构体匹配
    pusha               ; edi, esi, ebp, esp, ebx, edx, ecx, eax
    push ds
    push es
    push fs
    push gs

    ; 设置内核数据段
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; 根据新的栈布局获取中断号
    ; 栈布局: gs(4)+fs(4)+es(4)+ds(4)+pusha(32)=48字节
    mov eax, [esp+48]   ; 获取中断号

    ; 根据中断号调用对应的C处理函数
    cmp eax, 0
    je .call_divide_zero
    cmp eax, 13
    je .call_general_protection
    cmp eax, 32
    je .call_timer
    cmp eax, 33
    je .call_keyboard
    jmp .call_default

.call_divide_zero:
    push esp            ; 传递栈帧指针给C函数
    call divide_by_zero_handler
    add esp, 4
    jmp .done

.call_general_protection:
    push esp            ; 传递栈帧指针给C函数
    call general_protection_fault_handler
    add esp, 4
    jmp .done

.call_timer:
    call timer_interrupt_handler
    jmp .done

.call_keyboard:
    call keyboard_interrupt_handler
    jmp .done

.call_default:
    push esp
    call default_exception_handler
    add esp, 4

.done:
    ; 恢复寄存器
    pop gs
    pop fs
    pop es
    pop ds
    popa

    add esp, 8          ; 跳过int_no和err_code

    sti
    iret