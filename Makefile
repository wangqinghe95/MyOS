CC = gcc
LD = ld
ASM = nasm
QEMU = qemu-system-x86_64

INCLUDES = 

CFLAGS = -m32 -ffreestanding -nostdlib -c -I drivers -I kernel
ASFLAGS = -f elf32
LDFLAGS = -m elf_i386 -T scripts/linker.ld -nostdlib

OS_IMAGE = myos.img
BOOT_BIN = boot/boot.bin
BOOT_SRC = boot/boot.asm

KERNEL_ENTRY_SRC = kernel/entry.asm
KERNEL_ENTRY_OBJ = kernel/entry.o

KERNEL_C_SRC = kernel/kernel.c
KERNEL_C_OBJ = kernel/kernel.o

TYPE_H = kernel/types.h

# ========== 屏幕驱动 ==========
SCREEN_H = drivers/screen.h
SCREEN_C_SRC = drivers/screen.c
SCREEN_C_OBJ = drivers/screen.o

# ========== 中断相关 ==========
INTERRUPT_H = kernel/interrupt.h
INTERRUPT_C_SRC = kernel/interrupt.c
INTERRUPT_C_OBJ = kernel/interrupt_c.o

INTERRUPT_ASM_SRC = kernel/interrupt.asm
INTERRUPT_ASM_OBJ = kernel/interrupt_asm.o

# ========== 新增定时器驱动 ==========
TIMER_H = drivers/timer.h
TIMER_C_SRC = drivers/timer.c
TIMER_C_OBJ = drivers/timer.o

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

# ========== 更新链接命令，添加定时器对象 ==========
$(KERNEL_ELF): $(KERNEL_ENTRY_OBJ) $(KERNEL_C_OBJ) $(SCREEN_C_OBJ) $(INTERRUPT_ASM_OBJ) $(INTERRUPT_C_OBJ) $(TIMER_C_OBJ)
	$(LD) $(LDFLAGS) -o $(KERNEL_ELF) $(KERNEL_ENTRY_OBJ) $(KERNEL_C_OBJ) $(SCREEN_C_OBJ) $(INTERRUPT_ASM_OBJ) $(INTERRUPT_C_OBJ) $(TIMER_C_OBJ)

# ========== 新增定时器编译规则 ==========
$(TIMER_C_OBJ): $(TIMER_C_SRC) $(TIMER_H) $(SCREEN_H) $(TYPE_H)
	$(CC) $(CFLAGS) $(TIMER_C_SRC) -o $(TIMER_C_OBJ)

# ========== 中断汇编编译规则 ==========
$(INTERRUPT_ASM_OBJ): $(INTERRUPT_ASM_SRC)
	$(ASM) $(ASFLAGS) $(INTERRUPT_ASM_SRC) -o $(INTERRUPT_ASM_OBJ)

# ========== 中断C文件编译规则 ==========
$(INTERRUPT_C_OBJ): $(INTERRUPT_C_SRC) $(INTERRUPT_H) $(TYPE_H) $(SCREEN_H) $(TIMER_H)
	$(CC) $(CFLAGS) $(INTERRUPT_C_SRC) -o $(INTERRUPT_C_OBJ)

# ========== 更新kernel.c依赖，添加定时器头文件 ==========
$(KERNEL_C_OBJ): $(KERNEL_C_SRC) $(SCREEN_H) $(TYPE_H) $(INTERRUPT_H) $(TIMER_H)
	$(CC) $(CFLAGS) $(KERNEL_C_SRC) -o $(KERNEL_C_OBJ)

# ========== 屏幕驱动编译规则 ==========
$(SCREEN_C_OBJ): $(SCREEN_C_SRC) $(SCREEN_H) $(TYPE_H)
	$(CC) $(CFLAGS) $(SCREEN_C_SRC) -o $(SCREEN_C_OBJ)

# ========== 内核入口汇编编译规则 ==========
$(KERNEL_ENTRY_OBJ): $(KERNEL_ENTRY_SRC)
	$(ASM) $(ASFLAGS) $(KERNEL_ENTRY_SRC) -o $(KERNEL_ENTRY_OBJ)

# ========== 更新清理规则，添加定时器对象文件 ==========
clean:
	rm -f  $(OS_IMAGE) $(BOOT_BIN) $(KERNEL_ELF) $(KERNEL_BIN) \
	       $(KERNEL_ENTRY_OBJ) $(KERNEL_C_OBJ) $(SCREEN_C_OBJ) \
	       $(INTERRUPT_ASM_OBJ) $(INTERRUPT_C_OBJ) $(TIMER_C_OBJ)

run: $(OS_IMAGE)
	$(QEMU) -drive format=raw,file=$(OS_IMAGE)

.PHONY: all clean run