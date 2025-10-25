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

    ; --- Initialize COM1 (0x3F8) for 38400 baud, 8N1 ---
    mov dx, 0x3F8      ; base port for COM1
    mov al, 0x00
    add dx, 1
    out dx, al         ; IER = 0 (disable interrupts)
    sub dx, 1

    mov dx, 0x3F8
    add dx, 3
    mov al, 0x80       ; LCR: set DLAB = 1 to set divisor
    out dx, al
    sub dx, 3

    mov dx, 0x3F8
    mov al, 3          ; divisor low byte (3 -> 38400 if base clock 115200)
    out dx, al
    inc dx
    mov al, 0          ; divisor high byte
    out dx, al
    dec dx

    mov dx, 0x3F8
    add dx, 3
    mov al, 0x03       ; LCR = 8 bits, no parity, 1 stop bit (clear DLAB)
    out dx, al
    sub dx, 3

    mov dx, 0x3F8
    add dx, 2
    mov al, 0xC7       ; FCR: enable FIFO, clear them, 14-byte threshold
    out dx, al
    sub dx, 2

    mov dx, 0x3F8
    add dx, 4
    mov al, 0x0B       ; MCR: IRQs enabled, RTS/DSR set (set OUT2, RTS, DTR)
    out dx, al
    sub dx, 4

    ; --- Print message to serial ---
    mov si, msg
.print_loop:
    lodsb               ; AL = [SI], SI++
    cmp al, 0
    je .hang

    mov bl, al          ; save character in BL because IN will clobber AL

    ; wait for transmitter holding register empty (LSR bit 5 = 0x20)
    mov dx, 0x3F8
    add dx, 5           ; LSR port
.wait_lsr:
    in al, dx
    test al, 0x20
    jz .wait_lsr

    mov al, bl          ; restore character
    mov dx, 0x3F8       ; data port
    out dx, al
    jmp .print_loop

.hang:
    cli
    hlt
    ; single HLT: CPU is halted. We intentionally do not loop back.

msg db "Hello, OS! Booted to serial from boot_serial.asm", 0

; pad to 510 bytes and write boot signature
times 510 - ($ - $$) db 0
dw 0xAA55
