# MyOS

## 简介

这是一个从零开始制作简单操作系统（MyOS）的练习仓库，包含最小的引导扇区示例与相关说明。

## 第一步

首先编写一个最小的 `boot.asm`，把它作为引导扇区（512 字节）加载并执行。

## 仓库内容概览

- `boot.asm`：最小的 BIOS 引导扇区示例，通过 BIOS int 0x10 在屏幕上打印文本（VGA/文本模式）。
- `boot_serial.asm`：最小的 BIOS 引导扇区示例，初始化 COM1 并通过串口输出文本（适用于无头/终端环境）。
- `BOOT_README.md`：简短的构建/运行说明与后续步骤建议。
- `RECORD.md`：更详细的笔记与解释（已包含串口输出的代码与注释）。

## 构建与运行

将汇编文件汇成原始 512 字节的二进制镜像（注意 `org 0x7C00`）：

```bash
nasm -f bin -o boot.bin boot.asm
```

在 QEMU 中运行（带窗口的默认显示）：

```bash
qemu-system-x86_64 -drive format=raw,file=boot.bin
```

如果希望在当前终端（无图形）查看输出，可使用串口示例并将串口/监控器绑定到 stdio：

```bash
nasm -f bin -o boot_serial.bin boot_serial.asm
qemu-system-x86_64 -nographic -serial mon:stdio -drive format=raw,file=boot_serial.bin
```

注意：
- 使用 `-serial mon:stdio` 或 `-serial stdio` 时要小心，若 QEMU 的监控器或其它字符设备也占用了 stdio，会出现“cannot use stdio by multiple character devices”之类的错误。遇到此类错误，可改为 `-serial file:serial.log` 把输出重定向到文件，或使用 `-monitor none` 组合以避免冲突。
- `-nographic` 会禁用图形窗口并将虚拟机控制台路由到当前终端。

## 进阶/下一步建议

- 学习如何使用 BIOS int 0x13 从磁盘读取更多扇区并实现二阶段引导加载器。
- 学习如何设置 GDT 并切换到 32 位保护模式（以便加载 32 位内核）。
- 在引导加载器准备好后，用 C 语言编写一个小型内核并实现从引导加载器跳转到内核执行。

如果你希望我现在在当前环境中汇编并运行示例（我可以使用 `-serial mon:stdio` 或将串口输出保存到文件以避免 stdio 冲突），告诉我你偏好哪种方式，我会马上执行并把结果贴回。