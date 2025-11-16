# 工具定义
CC = gcc
LD = ld
ASM = nasm
OBJCOPY = objcopy
QEMU = qemu-system-x86_64

# 目录结构
BOOT_DIR = boot
KERNEL_DIR = kernel
DRIVERS_DIR = drivers
SCRIPT_DIR = scripts

# 编译和链接标志
CFLAGS = -m32 -nostdlib -ffreestanding -Wall -Wextra -I$(KERNEL_DIR) -I$(DRIVERS_DIR)
LDFLAGS = -m elf_i386 -T $(SCRIPT_DIR)/linker.ld -nostdlib
ASFLAGS = -f elf32

# 自动查找源文件（使用更精确的查找）
KERNEL_C_SRCS = $(shell find $(KERNEL_DIR) -name "*.c" -not -name ".*")
DRIVER_C_SRCS = $(shell find $(DRIVERS_DIR) -name "*.c" -not -name ".*")
KERNEL_ASM_SRCS = $(shell find $(KERNEL_DIR) -name "*.asm" -not -name ".*")

# 推导目标文件（使用不同的命名避免冲突）
KERNEL_C_OBJS = $(KERNEL_C_SRCS:.c=.c.o)
DRIVER_C_OBJS = $(DRIVER_C_SRCS:.c=.c.o)
KERNEL_ASM_OBJS = $(KERNEL_ASM_SRCS:.asm=.asm.o)

# 正确的链接顺序
ALL_OBJS = $(KERNEL_ASM_OBJS) $(KERNEL_C_OBJS) $(DRIVER_C_OBJS)

# 最终目标
KERNEL_ELF = $(KERNEL_DIR)/kernel.elf
KERNEL_BIN = $(KERNEL_DIR)/kernel.bin
OS_IMAGE = myos.img

# 默认目标
all: $(OS_IMAGE)

# 生成操作系统镜像
$(OS_IMAGE): $(BOOT_DIR)/boot.bin $(KERNEL_BIN)
	@echo "Creating OS image..."
	dd if=/dev/zero of=$@ bs=512 count=2880
	dd if=$(BOOT_DIR)/boot.bin of=$@ conv=notrunc
	dd if=$(KERNEL_BIN) of=$@ bs=512 seek=1 conv=notrunc
	@echo "OS image created: $@"

# 编译引导程序
$(BOOT_DIR)/boot.bin: $(BOOT_DIR)/boot.asm
	@echo "Building bootloader..."
	$(ASM) -f bin $< -o $@

# 生成内核二进制文件
$(KERNEL_BIN): $(KERNEL_ELF)
	@echo "Creating kernel binary..."
	$(OBJCOPY) -O binary $< $@

# 链接内核
$(KERNEL_ELF): $(ALL_OBJS)
	@echo "Linking kernel..."
	@echo "Object files: $(words $(ALL_OBJS)) files"
	@echo "Link command: $(LD) ... $(ALL_OBJS)"
	$(LD) $(LDFLAGS) -o $@ $(ALL_OBJS)
	@echo "Kernel linked: $@"

# 编译规则 - 使用不同的后缀避免冲突
%.c.o: %.c
	@echo "Compiling C: $< -> $@"
	$(CC) $(CFLAGS) -c $< -o $@

%.asm.o: %.asm
	@echo "Assembling: $< -> $@"
	$(ASM) $(ASFLAGS) $< -o $@

# 清理构建产物
clean:
	@echo "Cleaning build files..."
	rm -f $(OS_IMAGE) $(BOOT_DIR)/boot.bin $(KERNEL_BIN) $(KERNEL_ELF)
	find $(KERNEL_DIR) $(DRIVERS_DIR) -name "*.c.o" -delete
	find $(KERNEL_DIR) $(DRIVERS_DIR) -name "*.asm.o" -delete

# 显示详细的项目结构
debug:
	@echo "=== Build Configuration ==="
	@echo "Kernel ASM sources:"
	@for file in $(KERNEL_ASM_SRCS); do echo "  $$file"; done
	@echo "Kernel C sources:"
	@for file in $(KERNEL_C_SRCS); do echo "  $$file"; done
	@echo "Driver C sources:"
	@for file in $(DRIVER_C_SRCS); do echo "  $$file"; done
	@echo "All objects:"
	@for obj in $(ALL_OBJS); do echo "  $$obj"; done


run: $(OS_IMAGE)
	@echo "Starting QEMU..."
	$(QEMU) -drive format=raw,file=$(OS_IMAGE)

.PHONY: all clean run debug