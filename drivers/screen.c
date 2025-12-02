#include "screen.h"
#include <stdarg.h>

/* VGA 文本模式内存地址 */
#define VIDEO_MEMORY 0xB8000

/* 屏幕尺寸 */
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

/* 当前光标位置 */
static uint16_t cursor_x = 0;
static uint16_t cursor_y = 0;

/* 清屏函数 */
void clear_screen(void) {
    uint16_t* video_mem = (uint16_t*)VIDEO_MEMORY;
    uint8_t color = make_color(WHITE, BLACK);
    uint16_t blank = make_vga_entry(' ', color);
    
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        video_mem[i] = blank;
    }
    
    cursor_x = 0;
    cursor_y = 0;
    set_cursor_pos(0, 0);
}

/* 更新硬件光标位置 */
void set_cursor_pos(uint8_t x, uint8_t y) {
    uint16_t pos = y * SCREEN_WIDTH + x;
    
    /* 向 VGA 控制寄存器发送命令 */
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
    
    cursor_x = x;
    cursor_y = y;
}

/* 获取当前光标位置 */
uint16_t get_cursor_pos(void) {
    return cursor_y * SCREEN_WIDTH + cursor_x;
}

/* 滚屏 */
static void scroll(void) {
    uint16_t* video_mem = (uint16_t*)VIDEO_MEMORY;
    uint8_t color = make_color(WHITE, BLACK);
    uint16_t blank = make_vga_entry(' ', color);
    
    /* 将第2行到最后一行向上移动一行 */
    for (int i = 0; i < (SCREEN_HEIGHT - 1) * SCREEN_WIDTH; i++) {
        video_mem[i] = video_mem[i + SCREEN_WIDTH];
    }
    
    /* 清空最后一行 */
    for (int i = (SCREEN_HEIGHT - 1) * SCREEN_WIDTH; i < SCREEN_HEIGHT * SCREEN_WIDTH; i++) {
        video_mem[i] = blank;
    }
    
    cursor_y = SCREEN_HEIGHT - 1;
}

/* 输出单个字符 */
void put_char(char c, uint8_t color) {
    uint16_t* video_mem = (uint16_t*)VIDEO_MEMORY;
    
    /* 处理换行符 */
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } 
    /* 处理回车符 */
    else if (c == '\r') {
        cursor_x = 0;
    }
    /* 处理制表符 */
    else if (c == '\t') {
        cursor_x = (cursor_x + 4) & ~(4 - 1);
    }
    /* 处理普通字符 */
    else {
        uint16_t index = cursor_y * SCREEN_WIDTH + cursor_x;
        video_mem[index] = make_vga_entry(c, color);
        cursor_x++;
    }
    
    /* 检查是否需要换行或滚屏 */
    if (cursor_x >= SCREEN_WIDTH) {
        cursor_x = 0;
        cursor_y++;
    }
    
    if (cursor_y >= SCREEN_HEIGHT) {
        scroll();
    }
    
    /* 更新光标位置 */
    set_cursor_pos(cursor_x, cursor_y);
}

/* 输出字符串（默认颜色） */
void printk(const char* str) {
    printk_color(str, make_color(WHITE, BLACK));
}

/* 输出字符串（指定颜色） */
void printk_color(const char* str, uint8_t color) {
    while (*str) {
        put_char(*str++, color);
    }
}

/****

static void itoa(int value, char* str, int base) {
    char* ptr = str;
    char* ptr1 = str;
    char tmp_char;
    int tmp_value;
    
    // 处理0
    if (value == 0) {
        *ptr++ = '0';
        *ptr = '\0';
        return;
    }
    
    // 转换数字
    while (value) {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"[35 + (tmp_value - value * base)];
    }
    
    *ptr-- = '\0';
    
    // 反转字符串
    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
}

void printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    char buffer[32];
    
    while (*format) {
        if (*format == '%') {
            format++;
            switch (*format) {
                case 'd': {
                    int num = va_arg(args, int);
                    itoa(num, buffer, 10);
                    printk(buffer);
                    break;
                }
                case 's': {
                    char* str = va_arg(args, char*);
                    printk(str);
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
                    put_char(c, make_color(WHITE, BLACK));
                    break;
                }
                case 'x': {
                    int num = va_arg(args, int);
                    itoa(num, buffer, 16);
                    printk(buffer);
                    break;
                }
                default:
                    put_char(*format, make_color(WHITE, BLACK));
                    break;
            }
        } else {
            put_char(*format, make_color(WHITE, BLACK));
        }
        format++;
    }
    
    va_end(args);
}

 ****/