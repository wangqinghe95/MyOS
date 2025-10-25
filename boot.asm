; boot.asm - simple BIOS boot sector (512 bytes)
; Assembled with: nasm -f bin -o boot.bin boot.asm

org 0x7C00
bits 16

start:
    cli                 ; disable interrupts while setting up stack
    xor ax, ax
    mov ss, ax
    mov sp, 0x7C00      ; stack grows down from 0x7C00
    sti                 ; enable interrupts

    mov si, msg         ; pointer to message
.print_char:
    lodsb               ; al = [si], si++
    cmp al, 0
    je .hang
    mov ah, 0x0E        ; BIOS teletype function
    mov bh, 0x00        ; page
    mov bl, 0x07        ; color/attribute (for teletype this selects fg color)
    int 0x10
    jmp .print_char

.hang:
    cli
    hlt
    ; do not loop: single HLT to hang the CPU. If an interrupt occurs (shouldn't, because
    ; interrupts are disabled), execution could continue into the padding; we intentionally
    ; avoid an explicit jump here so the CPU remains halted instead of spinning.

msg db "Hello, OS! Booted from boot.asm", 0

; pad to 510 bytes so that signature is at offset 510-511
times 510 - ($ - $$) db 0
dw 0xAA55
