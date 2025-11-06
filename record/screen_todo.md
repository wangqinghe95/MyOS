# 屏幕驱动的功能完善

看了你的最新 `screen.c` 代码，功能已经相当完善了！以下是还需要完善的地方：

## 急需完善的功能

### 1. **硬件光标读取**（重要）
当前只能设置光标，无法读取当前硬件光标位置：

```c
/* 获取硬件光标位置 */
uint16_t get_hardware_cursor_pos(void) {
    outb(0x3D4, 0x0F);
    uint16_t pos = inb(0x3D5);
    outb(0x3D4, 0x0E);
    pos |= inb(0x3D5) << 8;
    return pos;
}

/* 需要添加 inb 函数 */
static inline uint8_t inb(uint16_t port) {
    uint8_t result;
    asm volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}
```

### 2. **光标可见性控制**
```c
void enable_cursor(bool enable) {
    outb(0x3D4, 0x0A);
    outb(0x3D5, enable ? 0x00 : 0x20); // 0x20 隐藏光标
}
```

### 3. **边界检查和安全处理**
```c
void put_char(char c, uint8_t color) {
    // 添加边界检查
    if (cursor_x >= SCREEN_WIDTH || cursor_y >= SCREEN_HEIGHT) {
        return; // 或者触发滚屏
    }
    
    // 安全检查
    if ((uint32_t)video_mem + index * 2 >= 0xB8000 + SCREEN_WIDTH * SCREEN_HEIGHT * 2) {
        return;
    }
    // ... 现有代码
}
```

## 功能增强

### 4. **屏幕区域管理**
```c
typedef struct {
    uint8_t start_x, start_y;
    uint8_t width, height;
    uint16_t cursor_x, cursor_y;
} screen_region;

void set_active_region(screen_region* region);
void printf_region(screen_region* region, const char* format, ...);
```

### 5. **格式化输出增强**
```c
// 支持更多格式符
case 'p': { // 指针
    void* ptr = va_arg(args, void*);
    printk("0x");
    itoa((uint32_t)ptr, buffer, 16);
    printk(buffer);
    break;
}
case 'f': { // 固定宽度数字
    int num = va_arg(args, int);
    int width = va_arg(args, int);
    itoa(num, buffer, 10);
    // 填充前导空格
    break;
}
```

### 6. **颜色主题支持**
```c
static uint8_t default_fg = WHITE;
static uint8_t default_bg = BLACK;

void set_default_color(uint8_t fg, uint8_t bg) {
    default_fg = fg;
    default_bg = bg;
}

void printk(const char* str) {
    printk_color(str, make_color(default_fg, default_bg));
}
```

## 性能优化

### 7. **批量输出优化**
```c
void printk_direct(const char* str, uint8_t color) {
    // 直接操作视频内存，减少函数调用
    uint16_t* video_mem = (uint16_t*)VIDEO_MEMORY;
    uint16_t entry = make_vga_entry(' ', color);
    
    while (*str && cursor_x < SCREEN_WIDTH && cursor_y < SCREEN_HEIGHT) {
        if (*str == '\n') {
            cursor_x = 0;
            cursor_y++;
        } else {
            video_mem[cursor_y * SCREEN_WIDTH + cursor_x] = make_vga_entry(*str, color);
            cursor_x++;
        }
        str++;
    }
    set_cursor_pos(cursor_x, cursor_y);
}
```

### 8. **滚屏优化**
```c
static void scroll(void) {
    uint16_t* video_mem = (uint16_t*)VIDEO_MEMORY;
    uint16_t blank = make_vga_entry(' ', make_color(WHITE, BLACK));
    
    // 使用memmove优化（如果实现了memmove）
    for (int i = 0; i < (SCREEN_HEIGHT - 1) * SCREEN_WIDTH; i++) {
        video_mem[i] = video_mem[i + SCREEN_WIDTH];
    }
    
    // 或者使用DMA（后期优化）
}
```

## 调试工具

### 9. **调试输出功能**
```c
void debug_printf(const char* format, ...) {
    static bool debug_enabled = true;
    if (!debug_enabled) return;
    
    uint16_t saved_x = cursor_x, saved_y = cursor_y;
    set_cursor_pos(0, SCREEN_HEIGHT - 1); // 最后一行用于调试
    
    va_list args;
    va_start(args, format);
    printf(format, args); // 需要创建 vprintf 变体
    va_end(args);
    
    set_cursor_pos(saved_x, saved_y);
}
```

### 10. **屏幕信息显示**
```c
void show_screen_info(void) {
    printf("Screen: %dx%d, Cursor: (%d,%d)\n", 
           SCREEN_WIDTH, SCREEN_HEIGHT, cursor_x, cursor_y);
}
```

## 错误处理

### 11. **错误状态返回**
```c
int put_char_safe(char c, uint8_t color) {
    if (cursor_y >= SCREEN_HEIGHT) {
        scroll();
        if (cursor_y >= SCREEN_HEIGHT) return -1; // 滚屏失败
    }
    // ... 正常逻辑
    return 0;
}
```

## 推荐实现顺序

**立即实现**（基础功能完整）：
1. 硬件光标读取 (`inb` + `get_hardware_cursor_pos`)
2. 边界检查和安全处理
3. 光标可见性控制

**近期实现**（功能增强）：
4. 格式化输出增强（%p, %f）
5. 颜色主题支持
6. 调试输出功能

**后期优化**：
7. 批量输出优化
8. 屏幕区域管理
9. 性能优化

你的屏幕驱动已经很不错了，现在可以优先实现**硬件光标读取**和**边界检查**，然后就可以开始开发中断系统了！