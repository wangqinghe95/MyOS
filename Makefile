CC = gcc
LD = ld
ASM = nasm
QEMU = qemu-system-x86_64

CFLAGS = -m32 -ffreestanding -nostdlib -c
ASFLAGS = -f elf32
LDFLAGS = -m elf_i386 -T scripts/linker.ld -nostdlib

OS_IMAGE = myos.img
BOOT_BIN = boot/boot.bin
BOOT_SRC = boot/boot.asm

KERNEL_ENTRY_SRC = kernel/entry.asm
KERNEL_ENTRY_OBJ = kernel/entry.o

KERNEL_C_SRC = kernel/kernel.c
KERNEL_C_OBJ = kernel/kernel.o

KERNEL_BIN = kernel/kernel.bin

all: $(OS_IMAGE)

$(OS_IMAGE): $(BOOT_BIN) $(KERNEL_BIN)
	dd if=/dev/zero of=$(OS_IMAGE) bs=512 count=2880
	dd if=$(BOOT_BIN) of=$(OS_IMAGE) conv=notrunc
	dd if=$(KERNEL_BIN) of=$(OS_IMAGE) bs=512 seek=1 conv=notrunc

$(BOOT_BIN): $(BOOT_SRC)
	$(ASM) -f bin $(BOOT_SRC) -o $(BOOT_BIN)

$(KERNEL_BIN): kernel/kernel.elf
	objcopy -O binary kernel/kernel.elf $(KERNEL_BIN)

kernel/kernel.elf: $(KERNEL_ENTRY_OBJ) kernel/kernel.o
	$(LD) $(LDFLAGS) -o kernel/kernel.elf $(KERNEL_ENTRY_OBJ) $(KERNEL_C_OBJ)

$(KERNEL_C_OBJ): $(KERNEL_C_SRC)
	$(CC) $(CFLAGS) $(KERNEL_C_SRC) -o $(KERNEL_C_OBJ)

$(KERNEL_ENTRY_OBJ): $(KERNEL_ENTRY_SRC)
	$(ASM) $(ASFLAGS) $(KERNEL_ENTRY_SRC) -o $(KERNEL_ENTRY_OBJ)

clean:
	rm -f *.o *.bin *.elf $(OS_IMAGE) $(BOOT_BIN) kernel/kernel.elf $(KERNEL_BIN) $(KERNEL_ENTRY_OBJ) $(KERNEL_C_OBJ)

run: $(OS_IMAGE)
	$(QEMU) -drive format=raw,file=$(OS_IMAGE)

.PHONY: all clean run