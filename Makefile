# Makefile for protected mode bootloader

# Tools
NASM := nasm
DD := dd
QEMU := qemu-system-i386

# Files
BOOT_BIN := boot.bin
DISK_IMG := disk.img

# Targets
all: $(DISK_IMG)

# Assemble bootloader
$(BOOT_BIN): boot.asm
	$(NASM) -f bin -o $@ $<

# Create disk image (1MB) and write bootloader
$(DISK_IMG): $(BOOT_BIN)
	$(DD) if=/dev/zero of=$@ bs=512 count=2048
	$(DD) if=$(BOOT_BIN) of=$@ conv=notrunc

# Run in QEMU (no graphic console)
run: $(DISK_IMG)
	$(QEMU) -drive file=$(DISK_IMG),format=raw -nographic

# Run in QEMU with VGA output
run-vga: $(DISK_IMG)
	$(QEMU) -drive file=$(DISK_IMG),format=raw

# Clean build artifacts
clean:
	rm -f $(BOOT_BIN) $(DISK_IMG)

.PHONY: all run run-vga clean
