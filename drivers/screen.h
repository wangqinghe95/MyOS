#ifndef SCREEN_H
#define SCREEN_H

#include "types.h"

/* 屏幕颜色枚举 */
typedef enum {
    BLACK = 0,
    BLUE = 1,
    GREEN = 2,
    CYAN = 3,
    RED = 4,
    MAGENTA = 5,
    BROWN = 6,
    LIGHT_GRAY = 7,
    DARK_GRAY = 8,
    LIGHT_BLUE = 9,
    LIGHT_GREEN = 10,
    LIGHT_CYAN = 11,
    LIGHT_RED = 12,
    LIGHT_MAGENTA = 13,
    YELLOW = 14,
    WHITE = 15
} vga_color;

// extern void outb(uint16_t port, uint8_t value);

/* 端口输出函数 */
static inline void outb(uint16_t port, uint8_t value) {
    asm volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t result;
    asm volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

/* 组合颜色字节 */
static inline uint8_t make_color(vga_color fg, vga_color bg) {
    return fg | (bg << 4);
}

/* 组合字符和颜色 */
static inline uint16_t make_vga_entry(char c, uint8_t color) {
    return (uint16_t)c | ((uint16_t)color << 8);
}

/* 函数声明 */
void clear_screen(void);
void put_char(char c, uint8_t color);
// void printk(const char* str);
// void printf(const char* format, ...);
void printk_color(const char* str, uint8_t color);
void set_cursor_pos(uint8_t x, uint8_t y);
uint16_t get_cursor_pos(void);

#endif