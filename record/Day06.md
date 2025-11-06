# # 从零开始制作 MyOS（六）

今天的任务是：添加异常处理代码——除零操作。

除零操作的过程理解：
C代码中的除零代码被编译成汇编语言，然后CPU在执行的时候发现除数为0后，直接触发除零错误，然后保存上下文，关中断，去读取IDT设置的错误类型是否有对应匹配，如果有就跳转到该错误处理方式。

## 背景知识

### 什么是中断

1. 中断是CPU暂停当前执行流程，转去执行特定处理程序，完成后恢复原流程的机制。
2. 中断分为硬件中断和软件中断：
    + 硬件中断是由外部设备，比如鼠标，键盘等发出，PIC/APIC 触发；
    + 软件中断是由 CPU 内部事件触发，比如异常：指令执行过程中产生的错误或特殊清空；比如软中断：通过指令 INT n 指令主动触发中断

### 中断的作用

1. 实现设备异步通知
2. 处理紧急事件
3. 支持多任务调度
4. 提供系统调用接口

### 什么异常

异常指CPU在执行指令过程中检测到的特殊条件或错误情况，会自动触发中断处理。

异常分为故障，陷阱，中止

1. 故障：可修复，修复后可重新执行的异常：比如叶故障，段不存在，堆栈故障
2. 陷阱：用于调试，执行后可继续下条指令，比如断点，单步执行，溢出等
3. 中止：一种无法恢复的严重错误，通常需要终止程序，比如有双重故障，机器检查

### 如何处理异常

在保护模式下，异常通过中断描述符表（IDT） 来处理。IDT包含256个条目，每个条目描述一个中断或异常的处理程序

### IDT

中断描述符表（Interrupt Descriptor Table），是 x86 架构在保护模式下用于处理中断和异常的数据结构，定义了 256 个中断向量的处理程序

IDT内存布局：
+----------------+ 0x00000000
|    条目0       | ← 异常0：除零错误
+----------------+
|    条目1       | ← 异常1：调试异常  
+----------------+
|    条目2       | ← 异常2：NMI
+----------------+
|      ...       |
+----------------+
|    条目32      | ← IRQ0：定时器中断
+----------------+
|      ...       |
+----------------+
|    条目255     | ← 最后的中断向量
+----------------+


### 异常处理流程

除法指令 → 除数为0 → CPU自动保存状态 → 查找IDT[0] → 跳转到isr0 → 执行异常处理程序


## 代码

### 一、数据结构定义

1. 中断描述符条目 （IDT Entry）

```
struct idt_entry {
    uint16_t base_low;    // 处理程序地址低16位
    uint16_t selector;    // 代码段选择子（0x08 = 内核代码段）
    uint8_t zero;         // 保留，必须为0
    uint8_t flags;        // 类型属性（0x8E = 32位中断门）
    uint16_t base_high;   // 处理程序地址高16位
} __attribute__((packed));
```

关键参数说明：
+ selector: 0x08 - 指向GDT中的内核代码段
+ flags: 0x8E - 表示32位中断门，DPL=0（最高权限）

2. 中断帧 （Interrupt Frame）

```
struct interrupt_frame {
    uint32_t eip;        // 发生异常时的指令指针
    uint32_t cs;         // 代码段选择子
    uint32_t eflags;     // 标志寄存器状态
    uint32_t esp;        // 栈指针
    uint32_t ss;         // 栈段选择子
    uint32_t error_code; // 错误代码（某些异常才有）
};
```

当异常发生时，CPU会自动将寄存器值压入栈中，形成中断帧

### 二、IDT 初始化过程

```
void idt_init(void) {
    struct idt_ptr idtp;
    idtp.limit = (sizeof(struct idt_entry) * IDT_ENTRIES) - 1;
    idtp.base = (uint32_t)&idt;

    // 设置除零异常处理
    idt_set_gate(0, (uint32_t)isr0, 0x08, 0x8E);

    idt_load((uint32_t)&idtp);
    printf("IDT initialized with divide-by-zero handler\n");
}
```

关键步骤：

1. 设置IDT指针（limit和base）
2. 配置异常0的处理程序为isr0
3. 使用lidt指令加载IDT

### 三、汇编异常处理程序

```interrupt.asm

isr0:
    cli                 ; 关中断，防止嵌套异常
    pusha               ; 保存所有通用寄存器
    push esp            ; 传递栈指针（指向中断帧）
    call divide_by_zero_handler  ; 调用C处理程序
    ; 处理程序不应返回，如果返回则继续执行
    add esp, 4
    popa
    sti                 ; 开中断
    iret                ; 中断返回（不会执行到这里）

```

寄存器保存的意义：
1. pusha - 保存EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI
2. push esp - 将当前栈指针传递给C函数作为中断帧指针

### 四、C 语言处理异常程序

```
void divide_by_zero_handler(struct interrupt_frame* frame) {
    printf("\n=== DIVIDE BY ZERO EXCEPTION ===\n");
    printf("Instruction Pointer: 0x%x\n", frame->eip);
    printf("Code Segment: 0x%x\n", frame->cs);
    printf("Stack Pointer: 0x%x\n", frame->esp);
    printf("=== SYSTEM HALTED ===\n");
    
    // 彻底挂起系统
    asm volatile("cli");
    while(1) {
        asm volatile("hlt");
    }
}
```

调试信息的意义：

1. EIP - 显示触发异常的指令位置
2. CS - 显示代码段，验证运行在保护模式
3. ESP - 显示栈指针，帮助调试栈相关问题

## 开发经验总结

1. 一直遇到 Qemu 界面窗口闪烁，也就是系统一直在重启
+ 问题是在于 extern void idt_load(void) 声明的时候是不带参数的，而在 interrupt.asm 汇编中，使用了 `mov eax, [esp + 4]  ; 获取IDT指针` , IDT 参数加载缺失

## 其他常见问题

1. 栈帧结构不匹配 - C函数接收到错误参数
2. 段寄存器未设置 - 在保护模式下调用C函数前需要正确设置数据段


