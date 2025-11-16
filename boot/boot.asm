org 0x7C00
bits 16

start:
    ; 初始化段寄存器
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    ; 显示启动信息
    mov si, msg_loading
    call print_string

    ; 加载内核到 0x10000
    mov ax, 0x1000
    mov es, ax
    xor bx, bx
    
    mov ah, 0x02    ; 读扇区
    mov al, 32      ; 32个扇区
    mov ch, 0       ; 柱面0
    mov cl, 2       ; 扇区2
    mov dh, 0       ; 磁头0
    mov dl, 0x80    ; 驱动器
    int 0x13
    jc disk_error

    ; 切换到保护模式
    cli
    lgdt [gdt_descriptor]
    
    ; 启用A20
    in al, 0x92
    or al, 2
    out 0x92, al
    
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    
    jmp CODE_SEG:init_pm

disk_error:
    mov si, msg_error
    call print_string
    jmp $

print_string:
    lodsb
    test al, al
    jz .done
    mov ah, 0x0E
    int 0x10
    jmp print_string
.done:
    ret

bits 32
init_pm:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov esp, 0x90000
    
    ; 跳转到内核
    jmp 0x10000

; 数据区
msg_loading db "Booting...", 0xD, 0xA, 0
msg_error db "Disk error!", 0

; GDT
gdt_start:
    dq 0
gdt_code:
    dw 0xFFFF
    dw 0
    db 0
    db 10011010b
    db 11001111b
    db 0
gdt_data:
    dw 0xFFFF
    dw 0
    db 0
    db 10010010b
    db 11001111b
    db 0
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

times 510-($-$$) db 0
dw 0xAA55