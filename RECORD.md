# 从零开始制作 MyOS

## 开发环境

1. 操作系统：ubuntu22 （windows10 + VMware15pro + ubunut22 + qemu）
2. 编译器：gcc-multilib
3. 汇编器：nasm
4. 模拟器: QEMU
5. 版本控制: git

## 安装依赖

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

## 前置知识

1. x86 汇编语言：寄存器，实模式 vs 保护模式，中断和异常，CPU 特权级
2. C 语言编程
3. 硬件基础知识：
    + 引导过程：当你按下电源键时，发生了什么事
    + BIOS/UEFI：它们做了什么
    + 内存映射：硬件设备（如 VGA 显存）在内存中的位置

### 实模式 VS 保护模式

实模式是处理器的初始状态，能够将内存视为连续的，无保护的物理空间，能够通过简单的算术运算扩展寻址范围；而保护模式则是通过硬件强制实施内存访问策略，将物理内存抽象成虚拟地址空间后的一种内存访问模式。

实模式下，程序能直接物理地址访问，而内存访问的范围则是在 1MB，也就是 20 位地址线内，不支持多任务，也没有内存保护。
保护模式下，物理地址被抽象成虚拟内存，程序通过分段和分页访问，最高能访问到 4GB 的内存范围。

保护模式是现代操作系统的基础。

### 计算机启动过程

当计算机上电后，位于 SPI Flash ROM 中的 BIOS 程序会被运行，该程序的任务是初始化计算的硬件，并且寻找可引导设备，这个可引导设备就是我们要开发的操作系统。

BIOS 在扇区 0 中找到有效地可引导设备后，就会将 CPU 的控制权转移过去，执行可引导设备程序。

### 关于 BIOS 程序

1. BIOS 引导程序物理存储地址是在 SPI Flash ROM ，也就是 串行外设接口闪存只读存储器 ，这个存储器是焊接在主板上，容量一般为 16 MB ~ 32 MB，断电后不丢失数据。
2. 在现代计算机中，传统的 BIOS 被 UEFI，也叫做 统一可扩展固件接口替代，它的存储位置也是在 SPI Flash 芯片中。
3. 由硬件厂商开发，BIOS 厂商根据芯片厂商提供的规范来负责编写 BIOS 代码
4. BIOS 的任务
    + 上电自检：检查关键硬件，包括 CPU，内存，芯片组等；然后初始化系统管理总线（SMBus），并且验证硬件完整性和兼容性
    + 硬件初始化：设置 CPU 微代码更新，配置内存控制器和时序参数，初始化 PCIe 设备枚举，设置 USB，SATA 控制器
    + 运行时服务建立：创建中断向量表，建立 BIOS 数据区，提供系统调用接口（INT，13h 磁盘服务等）

## 第一步：制作一个最简单的操作系统内核

启动电脑时，BIOS 会做自检，然后找到第一个可以启动的设备，读取该设备的第一个扇区（512 字节），如果该扇区最后两个字节是 0x55 和 0xAA，BIOS 会认为这是一个有效的引导扇区，并将其加载到内存 0x7c00 处执行。

下面我们使用汇编程序编写一个最简单的引导程序，也就是一个 boot.asm ：（必须使用汇编语言）


```
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

```

```bash
# 编译汇编文件
nasm -f bin boot.asm -o boot.bin

# 使用 QEMU 运行
qemu-system-x86_64 boot.bin

# vscode 终端 ssh 执行 QEMU 结果
qemu-system-x86_64 -nographic -monitor none -serial mon:stdio -drive file=boot.bin,format=raw,index=0,if=floppy

qemu-system-x86_64 -nographic -serial mon:stdio -drive format=raw,file=boot_serial.bin
```

### 运行结果

1. 会出现一个 QEMU 的黑屏窗口
2. 打印出 “"Hello, OS! Booted from boot.asm"”

[qemu-system-x86_64 boot.bin](./Resources/pic-1-1.png)

### 代码详解

boot.asm 文件都是汇编指令，下面对指令和它背后的意义做一个简单介绍：

1. boot.asm

它是一个最小的 BIOS 引导扇区，做了以下工作：
+ 被 BIOS 加载到物理地址 0x0000:0x7C00（也就是线性地址 0x7C00）并从那里执行。
+ 在屏幕上打印一行文本（通过 BIOS int 0x10 teletype 服务）。
+ 进入 halt 循环停止执行。
+ 文件被填充到 512 字节并以 0x55AA 结尾（这是 BIOS 引导签名）。

代码设计步骤：
+ org 0x7C00 + bits 16：引导扇区在实模式下，并且 BIOS 把第一个扇区加载到 0x0000:0x7C00，因此必须让汇编器使用那个基址来生成正确的地址。
+ 填充到 512 字节并写入 0x55AA：满足 BIOS 的最小引导扇区约定。
+ 使用 BIOS int 服务（int 0x10）来打印：简单、兼容且不需要直接操作视频内存。
+ 设置栈：引导阶段没有默认可靠的栈，需要显式设置以免后续调用/中断出现问题。
+ 禁用/恢复中断（cli/sti）：在设置栈或初始化关键结构时防止中断打断（可以提高稳定性）。

2. `org 0x7C00`
+ 告诉汇编器，代码段在源代码中被认为是从线性地址 0x7C00 开始的（即 BIOS 把扇区加载到内存 0x0000:0x7C00）

3. `bits 16`
指示 nasm 生成 16-bit 实模式编码

4. `start`
程序入口点标签（实际 BIOS 会跳转到 0x7C00，所以这只是代码中便于引用的标签）

5. `xor ax, ax`
+ `ax` 寄存器是
+ `xor` 是一个清零指令，
+ `xor ax, ax` 将 AX 清零（AX = 0），等同于指令 `mov ax, 0`，这里为接下来设 SS = 0 做准备

6. `mov ss, ax`

+ `mov` 指令，汇编语言中的赋值指令，将后者的值赋值给前者
+ 为了设定栈使用的段，将 SS（栈段寄存器）设为 0x0000（因为 AX 清零）。
+ 注意：在实模式下修改 SS 要小心（最好在修改 SP 之前或配套操作）。

7. `mov sp, 0x7C00`
+ 把栈指针 SP 设为 0x7C00（栈从 0x0000:0x7C00 向下增长）。
+ 必须设置栈，否则函数/中断可能导致不可预期行为。
+ 把栈放在 0x7C00 是一种常见简单做法（和引导扇区加载地址一致），但要确保不覆盖自身代码/数据
+ 如果后续会加载第二阶段，可能选不同位置。

8. `sti`
+ 允许中断指令（Set Interrupt Flag）。
+ 一般和 cli 配套使用，在想要想让 BIOS/硬件中断产生的地方使用

9. `mov si, msg`
+ 将 msg 值赋值给 si 寄存器，msg 本质是一个地址值，是一个存储字符串区域的首地址。
+ 把 SI 指向数据标签 msg，用于字符串读取。

10. `.print_char`
+ 一个循环标签
+ `jmp .print_char` 配合该条指令实现循环功能

11. `lodsb`
+ 从 [DS:SI] 处加载字节到 AL，然后 SI++（DS 默认是 0x0000，且 org 保证 msg 地址正确）。
+ 是一种简洁的逐字节读取方式。

12. `cmp al, 0`
+ `cmp` 是一个比较指令；
+ 比较 al 寄存器的值是否为 0 .
+ 目的是检查是否为字符串结束符（这里用 0 作为结束符）。
+ 用于结束循环。

13. `je .hang`
+ 如果 AL==0，则跳到结束（hang）。

14. `mov ah, 0x0E`
+ ah 寄存器是
+ 设置 BIOS int 0x10 的功能号为 teletype 输出（TTY 输出字符到当前光标并前进）。
+ 必须设 AH 才能让 int 0x10 执行正确的子功能。

15. `mov bh, 0x00`
+ 设置页面号（page）。BIOS teletype 函数使用 BH 指定页号（通常 0）。
+ 通常设为 0，是标准做法。

16. `mov bl, 0x07`
+ BL 设置字体属性/颜色
+ 尽管对于 teletype（0x0E）在传统文本模式 BL 并非总必需，但设会更兼容某些 BIOS。

17. `int 0x10`
+ int 指令为 BIOS 中断指令
+ 调用 BIOS 视频中断，执行上面设置的 teletype 输出（输出 AL 中的字符）。
+ 必须使用 BIOS 中断才能在实模式下不直接操作显存也输出字符（更简单）。

18. `jmp .print_char`
作用：继续循环输出下一个字符。

19. 

```
.hang:
    cli
    hlt
```

+ 进入禁中断并执行 halt 指令，cpu 进入低功耗等待模式，然后跳回（确保 CPU 不会继续向下执行垃圾代码）。
+ 需要一个安全的结束点而不返回到随机内存。hlt 比不停循环省电；
+ cli+hlt 防止来自中断的唤醒（但会阻塞直到外部复位），加上 jump 可在某些环境下避免返回到可能是可执行的区域。

20. msg db "Hello, OS! Booted from boot.asm", 0
+ 定义以 0 结尾的字符串数据，打印时 lodsb 逐字节读取直到 0。

21. 
```
times 510 - ($ - $$) db 0
```

+ 把文件填充到偏移 510（即文件前 510 个字节有效，接下来 2 字节用来放签名）。
+ 必须让整个扇区达到 512 字节，使得签名位于正确偏移。

22. `dw 0xAA55`

+ 写入引导签名 0x55AA（注意小端序写入会在磁盘上以 55 AA 的顺序保存）。
+ BIOS 在尝试从介质引导时会检查每个扇区末尾的 0x55AA 来判定该扇区是否为引导扇区；缺失此签名通常导致 BIOS 忽略该镜像作为引导设备。
## 使用串口输出（boot_serial.asm）

在没有图形界面的环境下，推荐用串口输出调试和显示信息。下面详细介绍串口初始化涉及的寄存器和每一步的作用。

### 串口输出代码结构

```boot_serial.asm
start:
    cli
    xor ax, ax
    mov ss, ax
    mov sp, 0x7C00
    sti
    call init_serial         ; 初始化串口
    mov si, msg
.print_loop:
    lodsb
    cmp al, 0
    je .hang
    call serial_putchar      ; 发送 AL 到串口
    jmp .print_loop
.hang:
    cli
    hlt

; 串口初始化例程 (COM1, 38400 8N1)
init_serial:
    ; 设置 IER = 0 (禁用中断)
    mov dx, 0x3F8      ; COM1 base port
    mov al, 0x00
    add dx, 1
    out dx, al
    sub dx, 1
    ; 设置 DLAB = 1，准备设置分频器
    mov dx, 0x3F8
    add dx, 3
    mov al, 0x80       ; LCR: DLAB=1
    out dx, al
    sub dx, 3
    ; 设置波特率分频器 (38400)
    mov dx, 0x3F8
    mov al, 3          ; divisor low byte
    out dx, al
    inc dx
    mov al, 0          ; divisor high byte
    out dx, al
    dec dx
    ; 设置 LCR = 8N1 (8位，无校验，1停止位)
    mov dx, 0x3F8
    add dx, 3
    mov al, 0x03       ; LCR: DLAB=0, 8N1
    out dx, al
    sub dx, 3
    ; 启用 FIFO
    mov dx, 0x3F8
    add dx, 2
    mov al, 0xC7       ; FCR: 启用FIFO，清空，14字节阈值
    out dx, al
    sub dx, 2
    ; 设置 MCR (RTS/DSR/OUT2)
    mov dx, 0x3F8
    add dx, 4
    mov al, 0x0B       ; MCR: IRQs enabled, RTS/DSR set
    out dx, al
    sub dx, 4
    ret

; 串口发送单字符例程 (AL)
serial_putchar:
    push dx
    push ax
    mov dx, 0x3F8
    add dx, 5           ; LSR port
.wait_lsr:
    in al, dx
    test al, 0x20       ; 检查 THRE (发送寄存器空)
    jz .wait_lsr
    pop ax              ; 恢复要发送的字符到 AL
    mov dx, 0x3F8       ; 数据端口
    out dx, al
    pop dx
    ret

msg db "Hello, OS! Booted to serial from boot_serial.asm", 0
```

### 串口初始化步骤与寄存器说明

PC 的标准串口 COM1 基地址是 0x3F8，串口芯片（16550A）有多个寄存器，分别控制不同功能：

1. **数据端口 (Data Register, 0x3F8)**
   - 用于收发数据。写入一个字节即可发送。
2. **中断使能寄存器 (IER, 0x3F9)**
   - 控制串口中断。我们设置为 0，禁用所有串口中断。
3. **分频器锁存寄存器 (DLL/DLM, 0x3F8/0x3F9, 需 DLAB=1)**
   - 设置波特率。波特率 = 基准频率 / 分频值。常见基准频率为 115200Hz，分频值为 3，则波特率为 38400。
   - DLL (低字节) 写入 3，DLM (高字节) 写入 0。
4. **线路控制寄存器 (LCR, 0x3FB)**
   - 控制数据位、停止位、校验位和 DLAB 位。
   - DLAB=1 时可设置分频器，DLAB=0 时正常通信。
   - 设置为 0x03 表示 8位数据，无校验，1停止位（8N1）。
5. **FIFO 控制寄存器 (FCR, 0x3FA)**
   - 控制 FIFO 缓冲区。0xC7 启用 FIFO，清空缓冲，设置 14字节阈值。
6. **调制解调器控制寄存器 (MCR, 0x3FC)**
   - 控制 RTS/DSR/OUT2 等信号。0x0B 启用 IRQs，设置 RTS/DSR。
7. **线路状态寄存器 (LSR, 0x3FD)**
   - 只读。用于检测发送寄存器是否空（THRE 位，0x20）。发送前需轮询该位。

### 串口发送字符流程
- 发送字符前，先轮询 LSR 的 THRE 位，确保发送寄存器空。
- 然后将要发送的字符写入数据端口 (0x3F8)。
- 这样可以保证数据不会丢失。

### 编译和运行
```bash
nasm -f bin boot_serial.asm -o boot_serial.bin
qemu-system-x86_64 -nographic -serial mon:stdio -drive format=raw,file=boot_serial.bin
```

### 运行效果
- 如果一切正常，你会在终端看到：

  Hello, OS! Booted to serial from boot_serial.asm

- 如果没有输出，请检查 boot_serial.asm 是否正确生成、QEMU 参数是否正确、串口初始化代码是否有误。

---

**总结：**
- 串口输出适合无头环境、远程调试、嵌入式开发。
- 代码中每一步都对应串口芯片的硬件寄存器设置，理解这些寄存器有助于后续开发更复杂的 bootloader 和内核调试功能。


