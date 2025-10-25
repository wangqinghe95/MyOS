# Makefile - assemble and run the boot sector
NASM=nasm
NASMFLAGS=-f bin
QEMU=qemu-system-x86_64

all: boot.bin

boot.bin: boot.asm
	$(NASM) $(NASMFLAGS) -o boot.bin boot.asm

run: boot.bin
	$(QEMU) -drive format=raw,file=boot.bin

clean:
	rm -f boot.bin
