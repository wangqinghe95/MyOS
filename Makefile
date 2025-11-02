CC = gcc
LD = ld
ASM = nasm
QEMU = qemu-system-x86_64

INCLUDES = -I drivers

CFLAGS = -m32 -ffreestanding -nostdlib -c $(INCLUDES)
ASFLAGS = -f elf32
LDFLAGS = -m elf_i386 -T scripts/linker.ld -nostdlib

OS_IMAGE = myos.img
BOOT_BIN = boot/boot.bin
BOOT_SRC = boot/boot.asm

KERNEL_ENTRY_SRC = kernel/entry.asm
KERNEL_ENTRY_OBJ = kernel/entry.o

KERNEL_C_SRC = kernel/kernel.c
KERNEL_C_OBJ = kernel/kernel.o

SCREEN_H = drivers/screen.h
SCREEN_C_SRC = drivers/screen.c
SCREEN_C_OBJ = drivers/screen.o

KERNEL_BIN = kernel/kernel.bin
KERNEL_ELF = kernel/kernel.elf

all: $(OS_IMAGE)

$(OS_IMAGE): $(BOOT_BIN) $(KERNEL_BIN)
	dd if=/dev/zero of=$(OS_IMAGE) bs=512 count=2880
	dd if=$(BOOT_BIN) of=$(OS_IMAGE) conv=notrunc
	dd if=$(KERNEL_BIN) of=$(OS_IMAGE) bs=512 seek=1 conv=notrunc

$(BOOT_BIN): $(BOOT_SRC)
	$(ASM) -f bin $(BOOT_SRC) -o $(BOOT_BIN)

$(KERNEL_BIN): $(KERNEL_ELF)
	objcopy -O binary $(KERNEL_ELF) $(KERNEL_BIN)

$(KERNEL_ELF): $(KERNEL_ENTRY_OBJ) $(KERNEL_C_OBJ) $(SCREEN_C_OBJ)
	$(LD) $(LDFLAGS) -o $(KERNEL_ELF) $(KERNEL_ENTRY_OBJ) $(KERNEL_C_OBJ) $(SCREEN_C_OBJ)

$(KERNEL_C_OBJ): $(KERNEL_C_SRC) $(SCREEN_H)
	$(CC) $(CFLAGS) $(KERNEL_C_SRC) -o $(KERNEL_C_OBJ)

$(SCREEN_C_OBJ): $(SCREEN_C_SRC) $(SCREEN_H)
	$(CC) $(CFLAGS) $(SCREEN_C_SRC) -o $(SCREEN_C_OBJ)

$(KERNEL_ENTRY_OBJ): $(KERNEL_ENTRY_SRC)
	$(ASM) $(ASFLAGS) $(KERNEL_ENTRY_SRC) -o $(KERNEL_ENTRY_OBJ)

clean:
	rm -f  $(OS_IMAGE) $(BOOT_BIN) $(KERNEL_ELF) $(KERNEL_BIN) $(KERNEL_ENTRY_OBJ) $(KERNEL_C_OBJ) $(SCREEN_C_OBJ)

run: $(OS_IMAGE)
	$(QEMU) -drive format=raw,file=$(OS_IMAGE)

.PHONY: all clean run