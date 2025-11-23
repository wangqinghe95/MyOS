# MyOS

## 简介

这是一个从零开始制作简单操作系统（MyOS）的练习仓库，包含最小的引导扇区示例与相关说明。

## 参考资料

1. OSDev Wiki - 深入的实践指南
2. 《操作系统：设计与实现》 - Minix作者的作品
3. Linux 0.11源码 - 学习简单的现代OS设计

## 环境搭建

### 开发环境

1. 操作系统：ubuntu22 （windows10 + VMware15pro + ubunut22 + qemu）
2. 编译器：gcc-multilib
3. 汇编器：nasm
4. 模拟器: QEMU
5. 版本控制: git

### 安装依赖

ubuntu22 中：

``` bash
# 安装必要的工具链
sudo apt update
sudo apt install -y build-essential
sudo apt install -y qemu-system-x86 
sudo apt install -y nasm    # x86架构汇编器
sudo apt install -y gdb
sudo apt install -y git
sudo apt install -y mtools  # 用于制作磁盘镜像

# 安装交叉编译器（重要！避免使用宿主系统的libc）
sudo apt install -y gcc-multilib
```

## 构建与运行

```bash
make

make run
```

## 文件夹结构

├── boot
│   └── boot.asm
├── drivers
│   ├── keyboard.c
│   ├── keyboard.h
│   ├── screen.c
│   ├── screen.h
│   ├── timer.c
│   └── timer.h
├── kernel
│   ├── entry.asm
│   ├── interrupt.asm
│   ├── interrupt.c
│   ├── interrupt.h
│   ├── kernel.c
│   └── types.h
├── libs
├── Makefile
├── README.md
└── scripts
    └── linker.ld

## 进度

### [day01 一个简单的引导程序](./record/Day01.md)

使用汇编语言完成操作系统最简单的版本  —— 一个引导程序 boot.asm

### [day02 加载多个磁盘空间](./record/Day02.md)

引导程序最重要的任务之一：在系统开始启动之前加载足够的磁盘空间，供系统启动，加载进程所用。
学习：使用 BIOS int 0x13 从磁盘读取更多扇区并实现二阶段引导加载器

### [day03 从实模式切换到保护模式](./record/Day03.md)

引导程序初始化任务完成之后，需要从实模式切换到保护模式，以便更好，更安全，效率更高地使用计算机硬件
学习：设置 GDT 并切换到 32 位保护模式（以便加载 32 位内核）。

### [day04 编写内核代码](./record/Day04.md)

使用高级语言编写操作系统内核代码
学习：在引导加载器准备好后，用 C 语言编写一个小型内核并实现从引导加载器跳转到内核执行。

### [day05 屏幕驱动代码](./record/Day05.md)

后续开发计划

+ 光标控制
+ 颜色管理
+ 格式化输出
+ 屏幕缓冲区管理
+ 调试输出功能

+ 硬件光标读取
+ 光标可见性控制
+ 边界检查和安全
+ 屏幕区域管理
+ 格式化输出增强
+ 颜色主题支持
+ 批量输出优化
+ 滚屏优化
+ 屏幕信息显示
+ 错误状态返回

### 中断管理开发

1. [day06-1 中断代码——除零异常处理](./record/Day06-01.md)
2. [day06-2 中断代码——通用保护故障异常处理](./record/Day06-02.md)
3. [day06-3 中断代码——定时器中断开发](./record/Day06-03.md)
4. [day06-4 中断代码——键盘中断开发](./record/Day06-04.md)

后续开发计划：
+ 特殊键处理（Ctrl, Alt, Shift）
+ 键盘指示灯（CapsLock, NumLock）
+ 更复杂的命令行解析
+ 键盘布局支持（不同国家）
+ 构建简单的 shell
+ 简单的文本剪辑器

### 内存管理开发

1. 物理内存管理
    + 完整的内存大小检测

2. 虚拟内存管理
3. 高级内存管理


位图物理页面分配器 - 跟踪哪些物理页面被使用

kmalloc/kfree实现 - 内核堆管理

分页机制 - 虚拟内存支持

缺页处理 - 按需分页

## 进阶/下一步建议

### 物理内存管理

+ 内存检测（通过GRUB或多重引导）
+ 位图分配器
+ 页帧分配

### 虚拟内存管理

+ 分页机制
+ 页目录/页表设置

### 进程管理

+ 任务状态段
+ 进程调度
+ 上下文切换

### 系统调用

+ 软件中断方式
+ 基本的进程创建

### 文件系统

+ 简单的FAT或ext2实现
+ 块设备驱动

### 用户模式

+ 特权级切换
+ 系统调用门

## 开发任务表

阶段一：完善核心系统
进程管理 - kernel/process.c, kernel/scheduler.c

内存分页 - kernel/paging.c, kernel/memory.c

文件系统 - kernel/fs.c, drivers/ata.c

阶段二：设备驱动
键盘输入 - drivers/keyboard.c

硬盘驱动 - drivers/ata.c

鼠标支持 - drivers/mouse.c

阶段三：用户空间
系统调用 - kernel/syscall.c

ELF加载器 - kernel/elf.c

用户程序 - apps/ 目录