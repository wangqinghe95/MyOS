section .text
global _start

_start:
    mov esp, 0x90000  ; 设置栈指针
    extern kernel_main
    call kernel_main   ; 调用C内核
    hlt