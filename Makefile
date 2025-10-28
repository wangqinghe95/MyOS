CC = gcc
LD = ld
ASM = nasm

CFLAGS = -m32 -ffreestanding -nostdlib -c
ASFLAGS = -f elf32
LDFLAGS = -m elf_i386 -T linker.ld -nostdlib

all: os.img

os.img: boot.bin kernel.bin
	dd if=/dev/zero of=os.img bs=512 count=2880
	dd if=boot.bin of=os.img conv=notrunc
	dd if=kernel.bin of=os.img bs=512 seek=1 conv=notrunc

boot.bin: boot.asm
	$(ASM) -f bin boot.asm -o boot.bin

kernel.bin: kernel.elf
	objcopy -O binary kernel.elf kernel.bin

kernel.elf: entry.o kernel.o
	$(LD) $(LDFLAGS) -o kernel.elf entry.o kernel.o

kernel.o: kernel.c
	$(CC) $(CFLAGS) kernel.c -o kernel.o

entry.o: entry.asm
	$(ASM) $(ASFLAGS) entry.asm -o entry.o

clean:
	rm -f *.o *.bin *.elf os.img

run: os.img
	qemu-system-x86_64 -drive format=raw,file=os.img

.PHONY: all clean run