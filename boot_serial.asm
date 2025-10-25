; boot_serial.asm - BIOS boot sector that outputs to serial (COM1) instead of VGA
; Assembled with: nasm -f bin -o boot_serial.bin boot_serial.asm

org 0x7C00
bits 16


start:
    cli
    xor ax, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    call init_serial

    mov si, msg
.print_loop:
    lodsb               ; AL = [SI], SI++
    cmp al, 0
    je .hang
    call serial_putchar ; 发送 AL 到串口
    jmp .print_loop

.hang:
    cli
    hlt
    ; single HLT: CPU is halted. We intentionally do not loop back.

;---------------------------------------------
; 串口初始化例程 (COM1, 38400 8N1)
;---------------------------------------------
init_serial:
    ; 设置 IER = 0 (禁用中断)
    mov dx, 0x3F8      ; COM1 base port
    mov al, 0x00
    add dx, 1
    out dx, al
    sub dx, 1

    ; 设置 DLAB = 1，准备设置分频器
    mov dx, 0x3F8
    add dx, 3
    mov al, 0x80       ; LCR: DLAB=1
    out dx, al
    sub dx, 3

    ; 设置波特率分频器 (38400)
    mov dx, 0x3F8
    mov al, 3          ; divisor low byte
    out dx, al
    inc dx
    mov al, 0          ; divisor high byte
    out dx, al
    dec dx

    ; 设置 LCR = 8N1 (8位，无校验，1停止位)
    mov dx, 0x3F8
    add dx, 3
    mov al, 0x03       ; LCR: DLAB=0, 8N1
    out dx, al
    sub dx, 3

    ; 启用 FIFO
    mov dx, 0x3F8
    add dx, 2
    mov al, 0xC7       ; FCR: 启用FIFO，清空，14字节阈值
    out dx, al
    sub dx, 2

    ; 设置 MCR (RTS/DSR/OUT2)
    mov dx, 0x3F8
    add dx, 4
    mov al, 0x0B       ; MCR: IRQs enabled, RTS/DSR set
    out dx, al
    sub dx, 4
    ret

;---------------------------------------------
; 串口发送单字符例程 (AL)
;---------------------------------------------
serial_putchar:
    push dx
    push ax
    mov dx, 0x3F8
    add dx, 5           ; LSR port
.wait_lsr:
    in al, dx
    test al, 0x20       ; 检查 THRE (发送寄存器空)
    jz .wait_lsr
    pop ax              ; 恢复要发送的字符到 AL
    mov dx, 0x3F8       ; 数据端口
    out dx, al
    pop dx
    ret

msg db "Hello, OS! Booted to serial from boot_serial.asm", 0

; pad to 510 bytes and write boot signature
times 510 - ($ - $$) db 0
dw 0xAA55
