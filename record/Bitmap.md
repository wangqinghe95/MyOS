# Question

## Day07

### bitmap

#### Q1

+ 关于 0x100000 地址疑问，代码中在文件 memory.h 中定义了

```
#define KERNEL_HEAP_START   (0x100000)
#define KERNEL_HEAP_SIZE    (0x100000)
```

在 memory.c 中 也多次使用 0x100000，那么这个地址设定有什么前置条件吗，还是可以随便定义，在后续中哪些地方要受到这个地址数字的约束？

#### A1

+ 0x100000 是一个特殊的地址，作为内核链接地址和加载地址存在
    + 这个地址不是一个随意的选择，它背后是x86架构的历史和硬件设计。
    + 在早期的PC中，CPU只有20根地址线。 这意味着它能寻址的最大内存是 2^20 = 1,048,576 字节，也就是 1MB。
    + 地址范围是 0x00000 到 0xFFFFF。
+ 以该地址位分界线，低地址留给硬件兼容，划分给各个关键系统组件比如
    + 0x00000000 - 0x000003FF: 中断向量表 (1KB)
    + 0x00000400 - 0x000004FF: BIOS数据区 (256B)  
    + 0x00000500 - 0x00007BFF: 可用区域 (约29KB)
    + 0x00007C00 - 0x00007DFF: 引导扇区 (512B)
    + 0x00007E00 - 0x0009FFFF: 可用区域 (约608KB)
    + 0x000A0000 - 0x000FFFFF: 硬件映射区域 (384KB)
        - 0xA0000: VGA显示内存
        - 0xB8000: 文本模式显存 (你的screen.c在使用)
        - 0xC0000: BIOS扩展
    + 0x00100000 - 0x00EFFFFF: 扩展内存 (14MB) ← 我们的可用区域开始！
+ 高地址才分给操作系统和应用软件


#### Q2

+ boot.asm 中设定了磁盘大小，那么这个磁盘大小数字在整个系统中起哪些作用；Makefile 中指定的  QEMU_MEMORY ?= 64 是物理内存地址？这两者之间的差别又在哪里？

#### A2

+ 先说磁盘和内存条在物理硬件上的区别：
    + 磁盘是一种非易失性存储设备，主要用于长期存储数据和程序。说人话就是断电后数据不丢失；类型包括机械硬盘，固态硬盘，U盘，SD 卡，eMMC芯片都属于磁盘
    + 物理内存和磁盘相对应，断电后数据立即丢失。一般指的是 DRAM，俗称内存条，在主板的内存插槽中，多个 DRAM 芯片组装在一个电路板上，形成标准模块。

+ 硬件交互流程：
    1. 开机：CPU 从主板骨剑（BIOS/UEFI）地址开始取指执行
    2. 固件程序从磁盘的固定位置（第一个扇区）读取引导程序，将其拷贝到物理内存的（0x7c00）地址
    3. 加载内核：引导程序（现在在内存中运行）通过向磁盘控制器发送指令，将操作系统内核从磁盘读入到物理内存的更高地址（如 0x100000）
    4. 执行程序：当你运行一个应用（如 ls）：
        - CPU 执行内存中的内核代码。
        - 内核代码向磁盘控制器发送指令，将 ls 程序从磁盘读入到物理内存的某个空闲区域。
        - CPU 然后跳转到该区域，开始执行 ls 程序的指令。

#### Q3: Qemu 从哪里获得引导程序，磁盘地址，和物理内存空间的
1. Makefile 编译时通过参数 `-drive` 指定了镜像文件，并且通过下面的脚本命令创建一个完整的磁盘镜像文件：

```
# 1. 创建一个空的大文件（比如 1.44MB，模仿软盘）
dd if=/dev/zero of=os-image.img bs=512 count=2880

# 2. 将 boot.asm 编译好的引导程序（boot.bin）写入这个镜像文件的第一个扇区（开头512字节）
dd if=boot.bin of=os-image.img bs=512 count=1 conv=notrunc

# 3. 将你的操作系统内核（kernel.bin）写入同一个镜像文件，从第二个扇区开始
dd if=kernel.bin of=os-image.img bs=512 seek=1 conv=notrunc
```

2. qemu 执行运行命令时，QEMU 会模拟一个 x86 电脑，并将 os-image.img 这个文件模拟成这台电脑的主硬盘（或软盘）

***注意：***
磁盘大小 1.44MB 空间大小后期可能不够，需要逐步增加，增加的大致节点是；

1. 实现内存分页管理后（内核会显著增大）
2. 添加系统调用框架后（系统调用表、处理函数会增加代码）
3. 实现进程管理时（进程控制块、调度器等）
4. 开始开发文件系统时（需要空间存储文件系统结构）
5. 准备添加用户程序时（shell、工具程序等需要存储空间）

定期检查

```
# 在 Makefile 中添加检查规则
check-size:
    @echo "Kernel size:"
    @ls -lh kernel.bin
    @echo "Disk free sectors:"
    @echo $$((2880 - 1 - ($$(stat -c%s kernel.bin) + 511) / 512))
```

后续增加磁盘大小的判断

#### Q3 ：详解一下位图的设计思路以及关键代码的讲解；最好用物理地址来举例给我讲解一下

位图的设计思路：使用一个 1bit 来跟踪 1 个 4 KB 物理页面的使用状态

**1. 核心数据结构**

**位图数组 - 内存状态的"地图"**
```c
static uint8_t bitmap[BITMAP_SIZE] = {0};
```
- **作用**：每个bit代表一个4KB物理页面的使用状态
- **大小计算**：`BITMAP_SIZE = (总页面数 / 8) + 1`
- **示例**：64MB可用内存 = 16384个页面 → 位图大小 = 2048字节

#### **状态跟踪变量**
```c
static uint32_t total_frames = 0;    // 总页面数
static uint32_t used_frames = 0;     // 已使用页面数  
static uint32_t bitmap_start_addr = 0; // 位图自身的内存地址
```

---

**2. 初始化过程详解**

**`init_bitmap_allocator()` - 初始化流程**
```c
void init_bitmap_allocator(void) {
    // 1. 计算总页面数
    total_frames = USABLE_MEMORY / PAGE_SIZE;  // 64MB → 16384个页面
    
    // 2. 清空位图（所有页面标记为空闲）
    memset(bitmap, 0, BITMAP_SIZE);  // 所有bit设为0
    
    // 3. 标记位图自身占用的内存
    bitmap_start_addr = (uint32_t)&bitmap[0];  // 位图在内存中的位置
    uint32_t bitmap_end_addr = bitmap_start_addr + BITMAP_SIZE;
    
    // 4. 计算位图占用了哪些页面
    uint32_t first_bitmap_frame = (bitmap_start_addr - USABLE_MEM_START) / PAGE_SIZE;
    uint32_t last_bitmap_frame = (bitmap_end_addr - USABLE_MEM_START) / PAGE_SIZE;
    
    // 5. 标记这些页面为已使用
    for(uint32_t i = first_bitmap_frame; i <= last_bitmap_frame; i++) {
        if(i < total_frames) {
            set_bitmap(i);    // 设置对应bit为1
            used_frames++;    // 更新使用计数
        }
    }
}
```

**初始化后的内存状态示例**：
```
物理内存布局：
0x100000 - 0x100FFF: [页面0] 空闲
0x101000 - 0x101FFF: [页面1] 空闲  
...
0x107000 - 0x107FFF: [页面7] 空闲
0x108000 - 0x108FFF: [页面8] 位图数据 ← 已占用
0x109000 - 0x109FFF: [页面9] 位图数据 ← 已占用
...
0x1FFFFF - 0x1FFFFF: [最后一个页面] 空闲

位图状态：
bitmap[0] = 0b00000011  // 第0-1字节：页面0-15的状态
bitmap[1] = 0b10000000  // 页面8（位图自身）被标记为1
```

---

**3. 核心操作函数详解**

**位操作三剑客**

**`set_bitmap(uint32_t bit)` - 标记页面为已使用**
```c
void set_bitmap(uint32_t bit) {
    uint32_t byte_index = bit / 8;     // 找到对应的字节
    uint32_t bit_index = bit % 8;      // 找到字节内的位位置
    bitmap[byte_index] |= (1 << bit_index);  // 设置特定位
}
```
**举例**：设置页面5
- `byte_index = 5 / 8 = 0` (第0个字节)
- `bit_index = 5 % 8 = 5` (第5位)
- `1 << 5 = 0b00100000`
- `bitmap[0] |= 0b00100000` → 第5位变为1

**`clear_bitmap(uint32_t bit)` - 标记页面为空闲**
```c
void clear_bitmap(uint32_t bit) {
    uint32_t byte_index = bit / 8;
    uint32_t bit_index = bit % 8;  
    bitmap[byte_index] &= ~(1 << bit_index);  // 清除特定位
}
```
**举例**：清除页面5
- `~(1 << 5) = 0b11011111` 
- `bitmap[0] &= 0b11011111` → 第5位变为0

**`test_bitmap(uint32_t bit)` - 检查页面状态**
```c
uint32_t test_bitmap(uint32_t bit) {
    uint32_t byte_index = bit / 8;
    uint32_t bit_index = bit % 8;
    return (bitmap[byte_index] >> bit_index) & 1;  // 提取特定位
}
```

---

**4. 页面分配与释放**

**`allocate_frame()` - 分配物理页面**
```c
uint32_t allocate_frame(void) {
    for(uint32_t i = 0; i < total_frames; i++) {
        if(!test_bitmap(i)) {           // 找到第一个空闲页面
            set_bitmap(i);              // 标记为已使用
            used_frames++;              // 更新计数器
            
            // 计算对应的物理地址
            uint32_t physical_addr = USABLE_MEM_START + (i * PAGE_SIZE);
            return physical_addr;       // 返回物理地址
        }
    }
    return 0;  // 内存耗尽
}
```

**分配过程示例**：
1. 遍历位图，找到第一个bit为0的位置（比如索引2）
2. `set_bitmap(2)` → 标记为已使用
3. 计算物理地址：`0x100000 + (2 * 0x1000) = 0x102000`
4. 返回 `0x102000`

**`free_frame(uint32_t frame_index)` - 释放物理页面**
```c
void free_frame(uint32_t frame_index) {
    // 物理地址 → 页面索引
    uint32_t index = (frame_index - USABLE_MEM_START) / PAGE_SIZE;
    
    if(index < total_frames) {
        if(test_bitmap(index)) {        // 确保页面确实被使用
            clear_bitmap(index);        // 标记为空闲
            used_frames--;              // 更新计数器
        } else {
            printf("WARNING: Double free!\n");
        }
    }
}
```

**释放过程示例**：
1. 收到释放请求：`free_frame(0x105000)`
2. 计算索引：`(0x105000 - 0x100000) / 0x1000 = 5`
3. `clear_bitmap(5)` → 标记页面5为空闲

---

**5. 实际工作流程示例**

假设系统运行时的操作序列：

```
操作序列：
1. 初始化：所有页面空闲，位图全0
2. 分配页面A：找到索引0 → 返回0x100000
   位图[0] = 0b00000001
3. 分配页面B：找到索引1 → 返回0x101000  
   位图[0] = 0b00000011
4. 分配页面C：找到索引2 → 返回0x102000
   位图[0] = 0b00000111
5. 释放页面B：索引1 → 位图[0] = 0b00000101
6. 分配页面D：找到索引1（重用）→ 返回0x101000
   位图[0] = 0b00000111
```

**对应的内存状态**：
```
物理地址      页面索引   位图状态     说明
0x100000      0         bit0=1      页面A在使用
0x101000      1         bit1=1      页面D在使用（B释放后重用）
0x102000      2         bit2=1      页面C在使用
0x103000      3         bit3=0      空闲
0x104000      4         bit4=0      空闲
```

---

**6. 设计优势与特点**

**空间效率极高**
- 管理64MB内存只需约2KB位图
- 开销比例：2KB / 64MB ≈ 0.003%

*时间复杂度**
- **分配**：O(n) - 线性搜索，实际中很快（位操作快）
- **释放**：O(1) - 直接计算索引
- **查询**：O(1) - 直接位测试

**内存布局清晰**
```
0x100000 - 0x1FFFFF: 可用内存区域
    ↑
    └── 位图精确跟踪每个4KB块的状态
```

**安全特性**
- 边界检查：`if(byte_index < BITMAP_SIZE)`
- 双重释放检测：`if(test_bitmap(index))`
- 无效地址检测：`if(index < total_frames)`

---

**7. 后续优化方向**

#### **当前实现的改进点**：
1. **分配算法**：从"首次适应"改为"最佳适应"
2. **搜索优化**：记录最后一个分配位置
3. **大页面支持**：连续页面的分配
4. **统计增强**：跟踪分配模式

#### **扩展功能**：
1. **内存碎片整理**
2. **页面回收机制**  
3. **内存压力检测**
4. **调试信息增强**

这个位图分配器为你的操作系统提供了坚实的内存管理基础！下一步可以基于它实现更高级的kmalloc/kfree或者开始虚拟内存管理。