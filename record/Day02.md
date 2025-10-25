# 从零开始制作 MyOS（二）

## 目标
一个简单的，能打印出字符串的 boot 已经做完了，那么接下来做一个加载多个磁盘扇区的功能。

## 原因

BIOS 在找到一个可引导设备后，加载该引导设备的第一个扇区，也就是扇区号为 0 的扇区，这个扇区只有 512 字节，那么这点空间在扣除引导设备签名和初始化一些寄存器代码后，剩余就没什么空间了，此时就需要加载额外的磁盘扇区，去实现更复杂的功能。

### 磁盘扇区的功能拓展

1. 文件系统驱动
2. 硬件检测和初始化
3. 保护模式切换代码
4. 内核加载器

## 磁盘介绍

1. 一个磁盘由多个盘面组成，比如一个光盘，就是正反两个盘面。盘片的表面涂有磁性物质，这些磁性物质用来记录二进制数据。因为正反两面都可涂上磁性物质，故一个盘片可能会有两个盘面
2. 盘片是一个圆盘，被划分成一个个同心圆形状的磁道。每个磁盘又被由圆心处按照一定角度射出来的直线划分为一个个扇区，每个扇区
3. 每个盘面对应一个磁头。所有的磁头都是连在同一个磁臂上的，因此所有磁头只能“共进退”。所有盘面中相对位置相同的磁道组成柱面
4. 所以磁盘的物理地址分为：柱面号，盘面号，扇区号，这样通过这三个值就能定位任意一个磁盘块。

## 加载磁盘的步骤

1. 初始化环境，建立稳定的运行环境：cli，sti，段寄存器等
2. 设置磁盘参数，读取指定位置：dl，dh，ch，cl
3. 设置目标地址，指定内存加载位置：es:bx
4. 调用 BIOS 服务，执行磁盘读取动作：int 0x13（AH=0x02）
5. 错误检查，检查读取是否成功：jc

## 相关的参数

1. AH = 0x02：读扇区功能号
2. AL = 要读的扇区数（通常为 1）
3. CH = 柱面号（Cylinder）
4. CL = 扇区号（Sector，1开始）
5. DH = 磁头号（Head）
6. DL = 驱动器号（0x00=软盘，0x80=第一个硬盘）
7. ES:BX = 目标内存地址（读到哪里）

## 代码

```boot.asm
; boot.asm - BIOS boot sector with multi-sector loading
; Assembled with: nasm -f bin -o boot.bin boot.asm

org 0x7C00      ; BIOS加载引导扇区到内存0x7C00处
bits 16         ; 16位实模式代码

;============= 初始化阶段 =============
start:
    ; 初始化段寄存器和栈
    cli         ; 禁用中断(防止在设置过程中被中断)
    xor ax, ax  ; AX清零(比mov ax,0更高效)
    mov ds, ax  ; 数据段寄存器DS=0
    mov es, ax  ; 额外段寄存器ES=0
    mov ss, ax  ; 堆栈段寄存器SS=0
    mov sp, 0x7C00 ; 栈指针SP=0x7C00(向下增长)
    sti         ; 启用中断

    ;============= 磁盘加载阶段 =============
    call load_disk  ; 调用磁盘加载例程

    ; 显示加载信息
    mov si, loading_msg
    call print_string

    ; 成功加载后跳转到加载的代码
    mov si, success_msg
    call print_string
    jmp 0x0000:0x8000 ; 跳转到加载的第二阶段代码

;============= 磁盘加载函数 =============
load_disk:
    ; 设置磁盘读取参数(CHS模式)
    mov dl, 0x80    ; 驱动器号(0x80=第一硬盘)
    mov dh, 0       ; 磁头号
    mov ch, 0       ; 柱面号
    mov cl, 2       ; 起始扇区号(1-based)
    mov al, 4       ; 要加载的扇区数(4*512=2048字节)
    mov bx, 0x8000  ; 目标地址ES:BX=0x0000:0x8000
    
    ; 调用BIOS磁盘服务
    mov ah, 0x02    ; 功能号0x02=读扇区
    int 0x13        ; 调用BIOS磁盘中断
    
    ; 错误处理应紧跟int 0x13
    jc disk_error   ; 如果CF=1(出错)则跳转
    ret             ; 成功返回

;============= 错误处理 =============
disk_error:
    mov si, error_msg
    call print_string
    mov si, retry_msg
    call print_string
    
    ; 等待按键
    xor ah, ah      ; 功能号0=等待按键
    int 0x16        ; 调用BIOS键盘服务
    
    ; 尝试热重启
    int 0x19        ; BIOS重启服务

;============= 字符串打印函数 =============
print_string:
    lodsb           ; 加载SI指向的字符到AL，并递增SI
    test al, al     ; 测试AL是否为0(字符串结束)
    jz .done        ; 如果是则跳转到.done
    
    ; 使用BIOS显示字符功能
    mov ah, 0x0E    ; 功能号0x0E=显示字符
    mov bh, 0       ; 显示页面0
    int 0x10        ; 调用BIOS视频服务
    
    jmp print_string ; 继续处理下一个字符
.done:
    ret             ; 返回

;============= 数据区 =============
loading_msg db "Loading sectors... ", 0
success_msg db "OK", 0xD, 0xA, "Booting...", 0
error_msg db "Error!", 0xD, 0xA, 0
retry_msg db "Press any key to reboot", 0

;============= 引导签名 =============
times 510 - ($ - $$) db 0  ; 填充剩余空间(510字节)
dw 0xAA55                  ; 引导扇区签名(最后2字节)

```

## 编译步骤

```bash
nasm -f bin -o boot.bin boot.asm

# 创建磁盘镜像并写入引导扇区
dd if=/dev/zero of=disk.img bs=512 count=2048
dd if=boot.bin of=disk.img conv=notrunc

# 将代码写入后续扇区
# dd if=kernel.bin of=disk.img bs=512 seek=1 conv=notrunc

qemu-system-x86_64 -nographic -serial mon:stdio -drive format=raw,file=disk.img
```

## 运行结果

```
Booting from Hard Disk...
Loading sectors... OK
Booting...
```

如果能看到这样结果，就说明运行成功