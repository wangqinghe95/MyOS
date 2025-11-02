// 添加屏幕驱动头文件
#include "screen.h"

// 简单的内核主函数
void kernel_main(void) {

    outb(0x3D4, 0x0F);  // 选择光标位置低字节寄存器
    outb(0x3D5, 0x00);  // 设置光标位置为0
    
    outb(0x3D4, 0x0E);  // 选择光标位置高字节寄存器  
    outb(0x3D5, 0x00);  // 设置光标位置为0

    // 清屏
    clear_screen();
    
    
    // 测试基本输出
    printk("Welcome to My Operating System!\n");
    printk("This is a simple screen driver test.\n\n");
    
    // 测试颜色输出
    printk_color("Red Text\n", make_color(RED, BLACK));
    printk_color("Green Text\n", make_color(GREEN, BLACK));
    printk_color("Blue Text\n", make_color(BLUE, BLACK));
    printk_color("Yellow Text\n", make_color(YELLOW, BLACK));
    printk_color("Cyan Text\n", make_color(CYAN, BLACK));
    printk_color("Magenta Text\n", make_color(MAGENTA, BLACK));
    
    // 测试特殊字符
    printk("\nTesting special characters:\n");
    printk("Tab:\t'Hello'\t'World'\n");
    printk("New lines:\nLine 1\nLine 2\nLine 3\n");
    
    // 测试滚屏 - 使用简单输出
    printk("\nTesting scroll (filling the screen):\n");
    printk("Line 1: This is a test line to demonstrate scrolling.\n");
    printk("Line 2: This is a test line to demonstrate scrolling.\n");
    printk("Line 3: This is a test line to demonstrate scrolling.\n");
    // ... 继续添加更多行直到滚屏
    
    // 完成消息
    printk_color("\nScreen driver test completed successfully!\n", 
                 make_color(LIGHT_GREEN, BLACK));
    
    // 挂起系统
    while(1) {
        // 空循环，保持系统运行
    }
}